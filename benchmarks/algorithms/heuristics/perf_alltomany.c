#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "alltomany.h"
#include "topology.h"
#include "pami_transport.h"

using namespace std;

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

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct multibfs bfs;
    optiq_multibfs_init(bfs);

    if (world_rank == 0) {
	printf("Topology: \n");
	for (int i = 0; i < bfs.num_dims; i++) {
	    printf("%d ", bfs.size[i]);
	}
	printf("\n");
	printf("num_dims = %d, num_nodes = %d\n", bfs.num_dims, bfs.num_nodes);
    }

    int ratio = 64;
    int num_dests = world_size/ratio;

    int *dest_ranks = (int *) malloc (sizeof(int) * num_dests);

    for (int i = 0; i < num_dests; i++) {
	dest_ranks[i] = i * ratio + ratio/2;
    }

    int count = 1 * 1024 * 1024;

    if (argc > 1)
    {
	count = atoi(argv[1]) * 1024;
    }

    int send_bytes = count * num_dests;
    char *send_buf = (char *) malloc(send_bytes);
    for (int i = 0; i < send_bytes; i++) {
	send_buf[i] = i % 128;
    }

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);
    for (int i = 0; i < num_dests; i++) 
    {
	sendcounts[dest_ranks[i]] = count;
	sdispls[dest_ranks[i]] = i * count;
    }

    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int recv_bytes = 0;
    char *recv_buf = NULL;
    int *rdispls = NULL;

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    recv_bytes = world_size * count;
	    recv_buf = (char *) malloc(recv_bytes);
	    rdispls = (int *) malloc(sizeof(int) * world_size);

	    for (int i = 0; i < world_size; i++) 
	    {
		recvcounts[i] = count;
		rdispls[i] = i * count;
	    }
	}
    }

    uint64_t t0 = GetTimeBase();

    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    optiq_alg_heuristic_search_alltomany(complete_paths, num_dests, dest_ranks, &bfs);

    /*if (world_rank == 0) {
      optiq_path_print_paths(complete_paths);
      }*/

    MPI_Barrier(MPI_COMM_WORLD);

    struct optiq_schedule schedule;
    schedule.world_rank = world_rank;
    schedule.world_size = world_size;

    schedule.send_buf = send_buf;
    schedule.sendcounts = sendcounts;
    schedule.sdispls = sdispls;

    schedule.recv_buf = recv_buf;
    schedule.recvcounts = recvcounts;
    schedule.rdispls = rdispls;

    schedule.remaining_jobs = num_dests;
    schedule.expecting_length = recv_bytes;
    schedule.sent_bytes = 0;
    schedule.chunk_size = 256 * 1024;

    optiq_schedule_init(schedule);

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    optiq_pami_transport_init();
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    optiq_transport_info_init(pami_transport);

    pami_transport->sched = &schedule;
    pami_transport->sched->pami_transport = pami_transport;

    uint64_t t1 = GetTimeBase();

    //optiq_schedule_create (schedule, complete_paths);

    for (int nbytes = 256*1024; nbytes <= count; nbytes *= 2)
    {
	for (int i = 0; i < schedule.local_jobs.size(); i++)
	{
	    schedule.local_jobs[i].buf_length = nbytes;
	}

	if (world_rank == 0) {
	    printf("\nnbytes = %d\n", nbytes);
        }

	for (int chunk_size = 1024; chunk_size <= nbytes; chunk_size *=2)
	{
	    if (world_rank == 0) {
		printf("chunk_size = %d\n", chunk_size);
	    }

	    schedule.chunk_size = chunk_size;
	    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs, schedule.chunk_size);

	    schedule.remaining_jobs = num_dests;
	    schedule.expecting_length = nbytes * world_size;
	    schedule.sent_bytes = 0;
	    memset (schedule.recv_bytes, 0, sizeof (int) * world_size);

	    MPI_Barrier (MPI_COMM_WORLD);

	    uint64_t t2 = GetTimeBase();

	    optiq_pami_transport_execute (pami_transport);

	    uint64_t t3 = GetTimeBase();

	    MPI_Barrier(MPI_COMM_WORLD);

	    double max_t, t = (double)(t3 - t2)/1.6e3;

	    long int data_size = (long int) num_dests * world_size * nbytes;
	    gather_print_time(t2, t3, 1, data_size, world_rank);
	}
    }

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    char *test_buf = (char *) malloc (recv_bytes);

	    for (int i = 0; i < recv_bytes; i++) {
		test_buf[i] = i%128;
	    }

	    if (memcmp (test_buf, recv_buf, recv_bytes) != 0) {
		printf ("Rank %d Received invalid data\n", world_rank);
	    }
	}
    }

    if (world_rank == 0)
    {
	optiq_path_print_stat(complete_paths, bfs.num_nodes);

	printf("\nTest with MPI_Alltoallv\n");
    }

    int iters = 30;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int i = 0; i < iters; i++)
    {
	MPI_Alltoallv(send_buf, sendcounts, sdispls, MPI_BYTE, recv_buf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t end = GetTimeBase();

    long int data_size = (long int) num_dests * world_size * count;
    gather_print_time(start, end, iters, data_size, world_rank);

    MPI_Finalize();

    return 0;
}
