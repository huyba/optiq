#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "mtonbfs.h"
#include "topology.h"
#include "pami_transport.h"

using namespace std;

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)

void gather_print_time(uint64_t start, uint64_t end, int iters, long int nbytes, int world_rank)
{
    double elapsed_time = (double)(end - start)/1.6e3;
    double max_time = 0.0;

    MPI_Reduce (&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        max_time = max_time / iters;
        double bw = (double) nbytes / max_time / 1024 / 1024 * 1e6;
        printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", nbytes/1024/1024, max_time, bw);
    }
}

void mpi_alltoallv(int nbytes)
{
    int world_size, world_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int ratio = 64;
    int num_dests = world_size/ratio;
    
    void *sendbuf = malloc(nbytes * num_dests);
    void *recvbuf = malloc(nbytes * world_size);

    int *sendcounts = (int *)malloc(sizeof(int) * world_size);
    int *sdispls = (int *)malloc(sizeof(int) * world_size);
    int *recvcounts = (int *)malloc(sizeof(int) * world_size);
    int *rdispls = (int *)malloc(sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) {
	sendcounts[i] = 0;
	sdispls[i] = 0;
	recvcounts[i] = 0;
	rdispls[i] = 0;
    }

    int dest, source;

    /*At sending side*/
    for (int i = 0; i < num_dests; i++) {
	dest = i * ratio + ratio/2;
        sendcounts[dest] = nbytes;
        sdispls[i] = i * nbytes ;
    }

    /*At receiving side*/
    if (world_rank % ratio == ratio/2) {
	printf("receiving rank %d\n", world_rank);

	for (int i = 0; i < world_size; i++) {
	    recvcounts[i] = nbytes;
	    rdispls[i] = i * nbytes;
	}
    }

    int iters = 30;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int i = 0; i < iters; i++) 
    {
	MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_BYTE, recvbuf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t end = GetTimeBase();

    long int data_size = (long int) num_dests * world_size * nbytes;
    gather_print_time(start, end, iters, data_size, world_rank);

    free(sendbuf);
    free(recvbuf);

    free(sendcounts);
    free(sdispls);
    free(recvcounts);
    free(rdispls);
}

void flow_create(int world_rank, int *next_dest)
{
    if (world_rank == 0) {
	next_dest[0] = 1;
	next_dest[1] = 2;
    }
    if (world_rank == 1) {
	next_dest[0] = 3;
    }
    if (world_rank == 3) {
	next_dest[0] = 5;
	next_dest[1] = 1;
	next_dest[3] = 1;
    }
    if (world_rank == 5) {
	next_dest[0] = 7;
	next_dest[1] = 3;
	next_dest[3] = 3;
    }
    if (world_rank == 7) {
	next_dest[0] = 6;
	next_dest[1] = 5;
	next_dest[3] = 5;
    }
    if (world_rank == 6) {
	next_dest[0] = 4;
	next_dest[1] = 7;
	next_dest[2] = 4;
	next_dest[3] = 7;
    }
    if (world_rank == 4) {
	next_dest[0] = 2;
	next_dest[1] = 6;
	next_dest[2] = 2;
    }
    if (world_rank == 2) {
	next_dest[1] = 4;
    }
}

void build_next_dest(int world_rank, int *next_dest, std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
	for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
	{
	    if (complete_paths[i]->arcs[j].u == world_rank) 
	    {
		next_dest[i] = complete_paths[i]->arcs[j].v;
	    }
	}
    }
}

std::vector<struct path *> optiq_pami_alltoallv(void *send_buf, int *sendcounts, int *sdispls, void *recv_buf, int *recvcounts, int *rdispls, struct optiq_bulk *bulk)
{
    //uint64_t t0 = GetTimeBase();

    /*Start the configuration for the test*/
    int num_dims = 5;
    int size[5];
    optiq_topology_get_size_bgq(size);

    int world_rank = bulk->pami_transport->rank;
    int world_size = bulk->pami_transport->size;

    /*Get number of dests and dests*/
    int num_sources = world_size;
    int *source_ranks = (int *) malloc (sizeof(int) * num_sources);
    for (int i = 0; i < num_sources; i++) {
	source_ranks[i] = i;
    }

    int ratio = 64;
    int num_dests = world_size/ratio;

    int *dest_ranks = (int *) malloc (sizeof(int) * num_dests);

    for (int i = 0; i < num_dests; i++) {
        dest_ranks[i] = i * ratio + ratio/2;
    }


    uint64_t t0 = GetTimeBase();

    /*Calculate paths to move data*/
    std::vector<struct path *> complete_paths;
    complete_paths.clear();
    mton_build_paths(complete_paths, num_sources, source_ranks, num_dests, dest_ranks, bulk->bfs);

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t t1 = GetTimeBase();

    build_next_dest(world_rank, bulk->next_dest, complete_paths);

    uint64_t t2 = GetTimeBase();

    int num_jobs = num_dests;
    int expecting_length = world_size * 1024 * 1024;

    int *flow_id = bulk->flow_id;
    int *final_dest = bulk->final_dest;

    bool isSource = false, isDest = false;

    int index = 0;
    for (int i = 0; i < complete_paths.size(); i++) {
	if (complete_paths[i]->arcs.back().v == world_rank) {
	    isDest = true;
	}

	if (complete_paths[i]->arcs.front().u == world_rank) {
	    isSource = true;
	    flow_id[index] = i;
	    final_dest[index] = complete_paths[i]->arcs.back().v;
	    index++;
	}
    }

    bulk->remaining_jobs = num_jobs;
    bulk->expecting_length = expecting_length;
    bulk->sent_bytes = 0;

    bulk->send_mr.offset = 0;
    bulk->recv_mr.offset = 0;

    bulk->rdispls = rdispls;

    bulk->isDest = isDest;

    uint64_t t3 = GetTimeBase();

    size_t bytes;
    pami_result_t result;
    int send_buf_size = 1 * 1024 * 1024;
    if (isSource) 
    {
	result = PAMI_Memregion_create (bulk->pami_transport->context, send_buf, send_buf_size, &bytes, &bulk->send_mr.mr);

	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < send_buf_size) {
	    printf("Registered less\n");
	}
    }

    int recv_buf_size = world_size * 1024 * 1024;
    if (isDest) 
    {
	result = PAMI_Memregion_create (bulk->pami_transport->context, recv_buf, recv_buf_size, &bytes, &bulk->recv_mr.mr);

	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < recv_buf_size) {
	    printf("Registered less\n");
	}
    }

    int nbytes = 32 * 1024;
    if (isSource) {
	for (int offset = 0; offset < send_buf_size; offset += nbytes) {
	    for (int i = 0; i < num_dests; i++) {
		struct optiq_message_header *header = bulk->pami_transport->extra.message_headers.back();
		bulk->pami_transport->extra.message_headers.pop_back();

		header->length = nbytes;
		header->source = world_rank;
		header->dest = final_dest[i];
		header->path_id = flow_id[i];

		memcpy(&header->mem, &bulk->send_mr, sizeof(struct optiq_memregion));
		header->mem.offset = offset;
		header->original_offset = offset;

		bulk->pami_transport->extra.send_headers.push_back(header);
	    }
	}
    }

    if (bulk->pami_transport->rank == 0) {
	double t = (double)(t3-t2)/1.6e3;
	printf("flow id, final dest %f\n", t);
	t = (double)(t2-t1)/1.6e3;
	printf("build next dest %f\n", t);
	t = (double)(t1-t0)/1.6e3;
        printf("create paths %f\n", t);
    }

    return complete_paths;
}

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct mtonbfs *bfs = (struct mtonbfs *) calloc (1, sizeof(struct mtonbfs));
    bfs->num_dims = 5;
    optiq_topology_get_size_bgq(bfs->size);
    bfs->num_nodes = 1;
    for (int i = 0; i < bfs->num_dims; i++) {
	bfs->num_nodes *= bfs->size[i];
    }

    bfs->neighbors = optiq_topology_get_all_nodes_neighbors(bfs->num_dims, bfs->size);

    if (world_rank == 0) {
	printf("num_dims = %d, num_nodes = %d\n", bfs->num_dims, bfs->num_nodes);
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    pami_transport->bulk.pami_transport = pami_transport;
    pami_transport->bulk.bfs = bfs;

    pami_transport->bulk.recv_bytes = (int *) calloc (1, sizeof(int) * world_size);
    pami_transport->bulk.final_dest = (int *) calloc (1, sizeof(int) * world_size);
    pami_transport->bulk.flow_id = (int *) calloc (1, sizeof(int) * world_size);

    pami_transport->bulk.next_dest = (int *) calloc (1, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

    optiq_pami_init_extra(pami_transport);
    optiq_pami_init(pami_transport);

    int ratio = 64;
    int num_dests = world_size/ratio;

    int *dests = (int *) malloc (sizeof(int) * num_dests);

    for (int i = 0; i < num_dests; i++) {
        dests[i] = i * ratio + ratio/2;
    }

    int send_bytes = 1 * 1024 * 1024;
    int send_buf_size = num_dests * send_bytes;
    char *send_buf = (char *) malloc(send_buf_size);
    for (int i = 0; i < send_buf_size; i++) {
	send_buf[i] = i % 128;
    }

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);
    for (int i = 0; i < num_dests; i++) 
    {
	sendcounts[dests[i]] = send_bytes;
	sdispls[dests[i]] = i * send_bytes;
    }

    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int recv_buf_size = 0;
    char *recv_buf = NULL;
    int *rdispls = NULL;

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dests[i]) 
	{
	    recv_buf_size = world_size * send_bytes;
	    recv_buf = (char *) malloc(recv_buf_size);
	    rdispls = (int *) malloc(sizeof(int) * world_size);

	    for (int i = 0; i < world_size; i++) 
	    {
		recvcounts[i] = send_bytes;
		rdispls[i] = i * send_bytes;
	    }
	}
    }

    uint64_t t0 = GetTimeBase();

    std::vector<struct path *> complete_paths = optiq_pami_alltoallv(send_buf, sendcounts, sdispls, recv_buf, recvcounts, rdispls, &pami_transport->bulk);

    uint64_t t1 = GetTimeBase();

    optiq_execute_jobs(pami_transport);

    uint64_t t3 = GetTimeBase();

    double max_t, t = (double)(t3 - t1)/1.6e3;

    long int data_size = (long int) num_dests * world_size * send_bytes;
    gather_print_time(t1, t3, 1, data_size, world_rank);

    for (int i = 0; i < num_dests; i++) {
	if (world_rank == dests[i]) {
	    char *test_buf = (char *) malloc (recv_buf_size);
	    for (int i = 0; i < recv_buf_size; i++) {
		test_buf[i] = i%128;
	    }
	    if (memcmp(test_buf, recv_buf, recv_buf_size) != 0) {
		printf("Rank %d Received invalid data\n", world_rank);
	    }
	}
    }

    if (world_rank == 0)
    {
	optiq_path_print_stat(complete_paths, bfs->num_nodes);
    }

    int nbytes = send_bytes;
    mpi_alltoallv(nbytes);

    MPI_Finalize();

    return 0;
}
