#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "manytomany.h"
#include "topology.h"
#include "pami_transport.h"
#include "cesm.h"

using namespace std;

struct optiq_comm_mem {
    char *send_buf;
    int send_len;
    int *sendcounts;
    int *sdispls;

    char *recv_buf;
    int recv_len;
    int *recvcounts;
    int *rdispls;
};

/* Allocate mem for send/recv, also assign counts, displacements*/
int optiq_comm_mem_allocate (std::vector<std::pair<int, std::vector<int> > > &source_dests, int count, struct optiq_comm_mem &comm_mem, int rank, int world_size)
{
    comm_mem.sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    comm_mem.sdispls = (int *)calloc(1, sizeof(int) * world_size);

    comm_mem.rdispls = (int *) malloc(sizeof(int) * world_size);
    comm_mem.recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int send_len = 0;
    int recv_len = 0;

    for (int i = 0; i < source_dests.size(); i++)
    {
	if (source_dests[i].first == rank)
	{
	    send_len = count * source_dests[i].second.size();
	    comm_mem.send_buf = (char *) malloc (send_len);

	    for (int j = 0; j < source_dests[i].second.size(); j++)
	    {
		int dest_id = source_dests[i].second[j];
		comm_mem.sendcounts[dest_id] = count;
		comm_mem.sdispls[dest_id] = i * count;
	    }
	}
    }
    comm_mem.send_len = send_len;

    int num_sources = 0;

    for (int i = 0; i < source_dests.size(); i++)
    {
	for (int j = 0; j < source_dests[i].second.size(); j++)
	{
	    if (source_dests[i].second[j] == rank)
	    {
		int source_id = source_dests[i].first;
		comm_mem.recvcounts[source_id] = count;
		comm_mem.rdispls[source_id] = num_sources * count;
		num_sources++;
	    }
	}
    }

    if (num_sources > 0) {
	recv_len = num_sources * count;
	comm_mem.recv_buf = (char *) malloc (recv_len);
    }
    comm_mem.recv_len = recv_len;

    return num_sources;
}

void optiq_comm_mem_delete(struct optiq_comm_mem &comm_mem)
{
    free(comm_mem.sendcounts);
    free(comm_mem.sdispls);
    if (comm_mem.send_len > 0) {
	free(comm_mem.send_buf);
    }

    free(comm_mem.recvcounts);
    free(comm_mem.rdispls);
    if (comm_mem.recv_len > 0) {
	free(comm_mem.recv_buf);
    }
}

void optiq_schedule_mem_destroy(struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
     size_t bytes;

    pami_result_t result;

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule.send_mr.mr);
    if (result != PAMI_SUCCESS)
    {
        printf("Destroy send_mr : No success\n");
    }

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule.recv_mr.mr);
    if (result != PAMI_SUCCESS)
    {
        printf("Destroy recv_mr : No success\n");
    }
}

void optiq_schedule_mem_reg (struct optiq_schedule &schedule, struct optiq_comm_mem &comm_mem, struct optiq_pami_transport *pami_transport)
{
    size_t bytes;

    pami_result_t result;

    result = PAMI_Memregion_create (pami_transport->context, comm_mem.send_buf, comm_mem.send_len, &bytes, &schedule.send_mr.mr);

    if (result != PAMI_SUCCESS)
    {
	printf("No success\n");
    }
    else if (bytes < comm_mem.send_len)
    {
	printf("Registered less\n");
    }

    result = PAMI_Memregion_create (pami_transport->context, comm_mem.recv_buf, comm_mem.recv_len, &bytes, &schedule.recv_mr.mr);

    if (result != PAMI_SUCCESS)
    {
	printf("No success\n");
    }
    else if (bytes < comm_mem.recv_len)
    {
	printf("Registered less\n");
    }
}

void gather_print_time(uint64_t start, uint64_t end, int iters, long int nbytes, int world_rank)
{
    double elapsed_time = (double)(end - start)/1.6e3;
    double max_time = 0.0;

    MPI_Reduce (&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    long int total_length = 0L;

    MPI_Reduce(&nbytes, &total_length, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
	max_time = max_time / iters;
	double bw = (double) total_length / max_time / 1024 / 1024 * 1e6;
	printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", total_length/1024/1024, max_time, bw);
    }
}

void optiq_schedule_set(struct optiq_schedule &schedule, int num_jobs, int world_size)
{
    schedule.remaining_jobs = num_jobs;
    schedule.expecting_length = schedule.recv_len;
    schedule.sent_bytes = 0;
    memset (schedule.recv_bytes, 0, sizeof (int) * world_size);
}

void optiq_schedule_execute(struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
    MPI_Barrier (MPI_COMM_WORLD);

    uint64_t t2 = GetTimeBase();

    optiq_execute_jobs (pami_transport);

    uint64_t t3 = GetTimeBase();

    MPI_Barrier(MPI_COMM_WORLD);

    double max_t, t = (double)(t3 - t2)/1.6e3;

    gather_print_time(t2, t3, 1, schedule.recv_len, pami_transport->rank);
}

void optiq_schedule_assign_job_demand(std::vector<struct optiq_job> &local_jobs, int nbytes)
{
    for (int i = 0; i < local_jobs.size(); i++) {
	local_jobs[i].buf_length = nbytes;
    }
}

void test_cesm_coupling (std::vector<int> &source_ranks, std::vector<int> &dest_ranks, int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    /*Create pairs of sources and destinations*/
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    optiq_cesm_gen_couple (source_ranks, dest_ranks, source_dests);

    /*Search for paths for each pair*/
    optiq_alg_heuristic_search_manytomany (complete_paths, source_dests, &bfs);

    /*With the comm pattern, allocate memories, set offsets, displacements*/
    struct optiq_comm_mem comm_mem;
    int num_sources = optiq_comm_mem_allocate (source_dests, count, comm_mem, pami_transport->rank, pami_transport->size);

    struct optiq_schedule schedule;
    schedule.world_size = pami_transport->size;
    schedule.world_rank = pami_transport->rank;
    schedule.pami_transport = pami_transport;
    pami_transport->sched = &schedule;

    optiq_schedule_init (schedule);

    /*Add paths and mem_comm to schedule*/
    optiq_schedule_add_paths (schedule, complete_paths);
    optiq_schedule_mem_reg (schedule, comm_mem, pami_transport);

    int demand = 1024 * 1024;
    int chunk_size = 256 * 1024;
    schedule.recv_len = demand * num_sources;

    /*Assign job size, chunk size and execute schedule*/
    optiq_schedule_assign_job_demand (schedule.local_jobs, demand);
    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs, chunk_size);
    optiq_schedule_set (schedule, dest_ranks.size(), pami_transport->size);
    optiq_schedule_execute (schedule, pami_transport);

    /*Deregister meme*/
    optiq_schedule_mem_destroy(schedule, pami_transport);

    optiq_schedule_finalize (schedule);

    /*Free the mem_comm*/
    optiq_comm_mem_delete (comm_mem);
}

void test_cesm(int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    int rank = pami_transport->rank;

    std::vector<int> cpl, land, ice, ocn, atm;

    optiq_cesm_gen(cpl, land, ice, ocn, atm, bfs.num_nodes);

    if (rank == 0) {
	printf ("\nFrom Coupler to Ice\n");
    }

    test_cesm_coupling(cpl, ice, count, bfs, pami_transport);

    if (rank == 0) {
	printf ("\nFrom Ice to Coupler\n");
    }

    test_cesm_coupling(ice, cpl, count, bfs, pami_transport);

    if (rank == 0) {
	printf ("\nFrom Coupler to Land\n");
    }

    test_cesm_coupling(cpl, land, count, bfs, pami_transport);

    if (rank == 0) {
	printf ("\nFrom Land to Coupler\n");
    }

    test_cesm_coupling(land, cpl, count, bfs, pami_transport);

    if (rank == 0) {
	printf ("\nFrom Coupler to Ocean\n");
    }

    test_cesm_coupling(cpl, ocn, count, bfs, pami_transport);

    if (rank == 0) {
	printf ("\nFrom Oceanto Coupler\n");
    }

    test_cesm_coupling(ocn, cpl, count, bfs, pami_transport);
}

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &world_size);

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

    int count = 1 * 1024 * 1024;

    if (argc > 1) {
	count = atoi (argv[1]) * 1024;
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *) calloc (1, sizeof(struct optiq_pami_transport));

    optiq_pami_init_extra (pami_transport);
    optiq_pami_init (pami_transport);

    test_cesm (count, bfs, pami_transport);

    MPI_Finalize();

    return 0;
}
