#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "manytomany.h"
#include "topology.h"
#include "pami_transport.h"
#include "comm_mem.h"
#include "schedule.h"

#include "cesm.h"

using namespace std;

int get_chunk_size(int num_nodes, int message_size)
{
    int chunk_size = 1;

    if (message_size <= 128 * 1024) {
	chunk_size = message_size;
    } else if (128 * 1024 < message_size && message_size <= 256 * 1024) {
	chunk_size = 128 * 1024;
    } else if (256 * 1024< message_size && message_size <= 512 * 1024) {
	chunk_size = 256 * 1024;
    } else if (512 * 1024 < message_size && message_size <= 1024 * 1024) {
	chunk_size = 512 * 1024;
    } else if (message_size > 1024 * 1024) {
	chunk_size = 1024 * 1024;
    } 

    return chunk_size;
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

void optiq_schedule_execute(struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
    MPI_Barrier (MPI_COMM_WORLD);

    uint64_t t2 = GetTimeBase();

    optiq_pami_transport_execute (pami_transport);

    uint64_t t3 = GetTimeBase();

    MPI_Barrier(MPI_COMM_WORLD);

    double max_t, t = (double)(t3 - t2)/1.6e3;

    gather_print_time(t2, t3, 1, schedule.recv_len, pami_transport->rank);
}

void test_cesm_coupling (std::vector<int> &source_ranks, std::vector<int> &dest_ranks, int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    /*Create pairs of sources and destinations*/
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    source_dests.clear();
    optiq_cesm_gen_couple (source_ranks, dest_ranks, source_dests);

    /*Search for paths for each pair*/
    optiq_alg_heuristic_search_manytomany_late_adding_load (complete_paths, source_dests, &bfs);

    /*With the comm pattern, allocate memories, set offsets, displacements*/
    struct optiq_comm_mem comm_mem;
    int num_sources = optiq_comm_mem_allocate (source_dests, count, comm_mem, pami_transport->rank, pami_transport->size);

    optiq_schedule_init();

    struct optiq_schedule schedule = *optiq_schedule_get();
    schedule.world_size = pami_transport->size;
    schedule.world_rank = pami_transport->rank;
    schedule.pami_transport = pami_transport;
    pami_transport->sched = &schedule;

    /* Add paths and mem_comm to schedule */
    optiq_schedule_add_paths (schedule, complete_paths);
    optiq_schedule_mem_reg (schedule, comm_mem, pami_transport);

    for (int demand = count; demand <= count; demand *= 2)
    {
	schedule.recv_len = demand * num_sources;

	/* Assign job size, chunk size and execute schedule */
	optiq_schedule_assign_job_demand (schedule.local_jobs, demand);

	for (int chunk_size = demand/4; chunk_size <= demand; chunk_size *= 2) 
	{
	    if (pami_transport->rank == 0) {
		printf("demand = %d chunk_size = %d ", demand, chunk_size);
	    }

	    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs, chunk_size);

	    optiq_schedule_set (&schedule, dest_ranks.size(), pami_transport->size);
	    optiq_schedule_execute (schedule, pami_transport);
	}
    }

    /*Deregister meme*/
    optiq_schedule_mem_destroy(&schedule, pami_transport);

    optiq_schedule_finalize ();

    /*Free the mem_comm*/
    optiq_comm_mem_delete (comm_mem);
}

void test_cesm(int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    int rank = pami_transport->rank;

    std::vector<int> cpl, land, ice, ocn, atm;

    optiq_cesm_gen (cpl, land, ice, ocn, atm, bfs.num_nodes);

    if (rank == 0) {
	printf ("\nFrom Oceanto Coupler\n");
    }

    test_cesm_coupling(ocn, cpl, count, bfs, pami_transport);

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
}

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init (&argc, &argv);

    MPI_Comm_rank (MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size (MPI_COMM_WORLD, &world_size);

    optiq_multibfs_init();

    struct multibfs bfs = *optiq_multibfs_get();

    if (world_rank == 0) {
	printf("Topology: \n");
	for (int i = 0; i < bfs.num_dims; i++) {
	    printf("%d ", bfs.size[i]);
	}
	printf("\n");
	printf("num_dims = %d, num_nodes = %d\n", bfs.num_dims, bfs.num_nodes);
    }

    int count = 2 * 1024 * 1024;

    if (argc > 1) {
	count = atoi (argv[1]) * 1024;
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    optiq_pami_transport_init ();
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    test_cesm (count, bfs, pami_transport);

    MPI_Finalize();

    return 0;
}
