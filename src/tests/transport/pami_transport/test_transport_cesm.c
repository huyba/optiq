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
}

void optiq_comm_mem_generate (struct optiq_comm_mem &comm_mem, int world_size, int send_len, int recv_len)
{
    comm_mem.send_len = send_len;
    comm_mem.send_buf = (char *) malloc (send_len);
    comm_mem.sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    comm_mem.sdispls = (int *)calloc(1, sizeof(int) * world_size);

    comm_mem.recv_len = recv_len;
    comm_mem.recv_buf = (char *) malloc (recv_len);
    comm_mem.rdispls = (int *) malloc(sizeof(int) * world_size);
    comm_mem.recvcounts = (int *) calloc(1, sizeof(int) * world_size);   
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

void test_cems_actual_execution(int nbytes, int chunk_size, int num_dests, int expecting_length, struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
    int world_rank = pami_transport->rank;
    int world_size = pami_transport->size;

    if (world_rank == 0) {
        printf("\nnbytes = %d\n", nbytes);
    }

    for (int i = 0; i < schedule.local_jobs.size(); i++) {
        schedule.local_jobs[i].buf_length = nbytes;
    }

    if (world_rank == 0) {
	printf("chunk_size = %d\n", chunk_size);
    }

    schedule.chunk_size = chunk_size;
    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs, schedule.chunk_size);

    schedule.remaining_jobs = num_dests;
    schedule.expecting_length = expecting_length;
    schedule.sent_bytes = 0;
    memset (schedule.recv_bytes, 0, sizeof (int) * world_size);

    MPI_Barrier (MPI_COMM_WORLD);

    uint64_t t2 = GetTimeBase();

    optiq_execute_jobs (pami_transport);

    uint64_t t3 = GetTimeBase();

    MPI_Barrier(MPI_COMM_WORLD);

    double max_t, t = (double)(t3 - t2)/1.6e3;

    gather_print_time(t2, t3, 1, expecting_length, world_rank);
}

void optiq_schedule_execute(struct optiq_schedule &schedule,  struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    int world_rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int send_bytes = 0;
    char *send_buf = NULL;
    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    int recv_bytes = 0;
    char *recv_buf = NULL;
    int *rdispls = (int *) malloc(sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int num_dests = dest_ranks.size();
    int num_sources = source_ranks.size();

    long int expecting_length = 0L;

    for (int i = 0; i < num_sources; i++)
    {
	if (world_rank == source_ranks[i])
	{
	    send_bytes = count * num_dests;
	    send_buf = (char *) malloc(send_bytes);

	    for (int j = 0; j < send_bytes; j++) {
		send_buf[j] = j % 128;
	    }

	    for (int j = 0; j < num_dests; j++)
	    {
		sendcounts[dest_ranks[j]] = count;
		sdispls[dest_ranks[j]] = j * count;
	    }
	}
    }

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    recv_bytes = num_sources * count;
	    recv_buf = (char *) malloc(recv_bytes);

	    for (int j = 0; j < num_sources; j++) 
	    {
		recvcounts[source_ranks[j]] = count;
		rdispls[source_ranks[j]] = j * count;
	    }

	    expecting_length = recv_bytes;
	}
    }

    uint64_t t0 = GetTimeBase();

    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    optiq_cesm_gen_couple(source_ranks, dest_ranks, source_dests);

    optiq_alg_heuristic_search_manytomany(complete_paths, source_dests, &bfs);

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

    optiq_schedule_init(schedule);

    pami_transport->sched = &schedule;
    pami_transport->sched->pami_transport = pami_transport;

    uint64_t t1 = GetTimeBase();

    optiq_schedule_create (schedule, complete_paths);

    //optiq_schedule_print_jobs(schedule);

    for (int nbytes = count; nbytes <= count; nbytes *= 2)
    {
	if (expecting_length != 0) {
	    expecting_length = num_sources * nbytes;
	}

	for (int chunk_size = count/4; chunk_size <= nbytes; chunk_size *=2) {
	    test_cems_actual_execution(nbytes, chunk_size, num_dests, expecting_length, schedule, pami_transport);
	}
    }

    char *test_buf = (char *) malloc (recv_bytes);;

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    for (int i = 0; i < recv_bytes; i++) {
		test_buf[i] = i%128;
	    }

	    if (memcmp (test_buf, recv_buf, recv_bytes) != 0) {
		printf ("Rank %d Received invalid data\n", world_rank);
	    }
	    memset(recv_buf, 0, recv_bytes);
	}
    }

    if (world_rank == 0)
    {
	optiq_path_print_stat(complete_paths, bfs.num_nodes);

	printf("\nTest with MPI_Alltoallv\n");
    }

    optiq_schedule_finalize(schedule);

    int iters = 20;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int i = 0; i < iters; i++)
    {
	MPI_Alltoallv(send_buf, sendcounts, sdispls, MPI_BYTE, recv_buf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t end = GetTimeBase();

    gather_print_time(start, end, iters, recv_bytes, world_rank);

    for (int i = 0; i < num_dests; i++)
    {
	if (world_rank == dest_ranks[i])
	{
	    for (int i = 0; i < recv_bytes; i++) {
		test_buf[i] = i%128;
	    }

	    if (memcmp (test_buf, recv_buf, recv_bytes) != 0) {
		printf ("Rank %d Received invalid data\n", world_rank);
	    }
	}
    }

    free(sendcounts);
    free(sdispls);
    free(rdispls);
    free(recvcounts);

    if (send_buf != NULL) {
	free(send_buf);
    }
    if (recv_buf != NULL) {
	free(recv_buf);
    }
    if (test_buf != NULL) {
	free(test_buf);
    }
}

void test_cesm_coupling(std::vector<int> &source_ranks, std::vector<int> &dest_ranks, int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    /*Create pairs of sources and destinations*/
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    optiq_cesm_gen_couple(source_ranks, dest_ranks, source_dests);

    /*Search for paths for each pair*/
    optiq_alg_heuristic_search_manytomany (complete_paths, source_dests, &bfs);

    /*With the comm pattern, allocate memories, set offsets, displacements*/
    optiq_mem_comm_create(source_dests, count, mem_comm);

    struct optiq_schedule schedule;
    optiq_schedule_init (schedule);

    /*Add paths and mem_comm to schedule*/
    optiq_schedule_add_paths (schedule, complete_paths);
    optiq_schedule_add_mem (schedule, mem_comm);

    int nbytes = 1024 * 1024;
    int chunk_size = 256 * 1024;

    /*Assign job size, chunk size and execute schedule*/
    optiq_schedule_assign_job_size(nbytes);
    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs,chunk_size);
    optiq_schedule_execute(schedule, bfs, pami_transport);

    /*Free the mem_comm*/
    optiq_mem_comm_delete(mem_comm);
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

    int count = 1 * 1024 * 1024;

    if (argc > 1) {
	count = atoi(argv[1]) * 1024;
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    optiq_pami_init_extra(pami_transport);
    optiq_pami_init(pami_transport);

    test_cesm(count, bfs, pami_transport);

    MPI_Finalize();

    return 0;
}
