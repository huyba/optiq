#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <math.h>
#include <unistd.h>
#include <mpi.h>

#include "manytomany.h"
#include "multipaths.h"
#include "topology.h"
#include "pami_transport.h"
#include "comm_mem.h"
#include "schedule.h"

#include "patterns.h"

using namespace std;

char *graphFilePath;

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

void test_mpi(struct optiq_comm_mem &comm_mem, struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
    int iters = 10;

    MPI_Barrier (MPI_COMM_WORLD);
        
    uint64_t t2 = GetTimeBase();

    for (int i = 0; i < iters; i++) {
	MPI_Alltoallv(comm_mem.send_buf, comm_mem.sendcounts, comm_mem.sdispls, MPI_BYTE, comm_mem.recv_buf, comm_mem.recvcounts, comm_mem.rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t t3 = GetTimeBase();
    
    MPI_Barrier(MPI_COMM_WORLD);

    if (pami_transport->rank == 0) {
	printf("MPI_Alltoallv ");
    }

    gather_print_time(t2, t3, iters, schedule.recv_len, pami_transport->rank);
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

void test_coupling (std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_jobs, int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    /*Search for paths for each pair*/
    //optiq_alg_heuristic_search_manytomany (complete_paths, source_dests, &bfs);
    int k = 2;
    optiq_alg_heuristic_search_kpaths(complete_paths, source_dests, k, graphFilePath);

    if (pami_transport->rank == 0) {
	optiq_path_print_stat(complete_paths, bfs.num_nodes);
    }

    /*With the comm pattern, allocate memories, set offsets, displacements*/
    struct optiq_comm_mem comm_mem;
    int num_sources = optiq_comm_mem_allocate (source_dests, count, comm_mem, pami_transport->rank, pami_transport->size);

    struct optiq_schedule schedule;
    schedule.world_size = pami_transport->size;
    schedule.world_rank = pami_transport->rank;
    schedule.pami_transport = pami_transport;
    pami_transport->sched = &schedule;

    optiq_schedule_init (schedule);

    /* Add paths and mem_comm to schedule */
    optiq_schedule_add_paths (schedule, complete_paths);
    optiq_schedule_mem_reg (schedule, comm_mem, pami_transport);

    for (int demand = count; demand <= count; demand *= 2)
    {
	schedule.recv_len = demand * num_sources;

	/* Assign job size, chunk size and execute schedule */
	optiq_schedule_assign_job_demand (schedule.local_jobs, demand);

	for (int chunk_size = 16 * 1024; chunk_size <= demand; chunk_size *= 2) 
	{
	    if (pami_transport->rank == 0) {
		printf("demand = %d chunk_size = %d ", demand, chunk_size);
	    }

	    optiq_schedule_split_jobs_multipaths (pami_transport, schedule.local_jobs, chunk_size);

	    optiq_schedule_set (schedule, num_jobs, pami_transport->size);
	    optiq_schedule_execute (schedule, pami_transport);
	}

	/* MPI_Alltoallv test*/
	test_mpi(comm_mem, schedule, pami_transport);
    }

    /*Deregister meme*/
    optiq_schedule_mem_destroy(schedule, pami_transport);

    optiq_schedule_finalize (schedule);

    /*Free the mem_comm*/
    optiq_comm_mem_delete (comm_mem);
}

void test_patterns(int count, struct multibfs &bfs, struct optiq_pami_transport *pami_transport)
{
    int rank = pami_transport->rank;

    std::vector<int> sources;
    std::vector<int> dests;
    std::vector<std::pair<int, std::vector<int> > > source_dests;

    int ratio = 0;

    for (int i = 1; i < 4; i++) 
    {
	ratio = pow(2, i) - 1;

	if (rank == 0) {
	    printf ("\nDisjoint - Contigous - Source:Dest ratio : %d : 1 c1\n", ratio);
	}

	disjoint_contigous (bfs.num_nodes, sources, dests, source_dests, ratio);
	test_coupling (source_dests, dests.size(), count, bfs, pami_transport);
    }

    for (int i = 1; i <= 6; i++)
    {
        ratio = pow(2, i);

	if (rank == 0) {
	    printf ("\nSubset - Uniformly distributed (IO Agg) - Source:Dest ratio : %d : 1 c1\n", ratio);
	}

	subset_udistributed_ioagg(bfs.num_nodes, sources, dests, source_dests, ratio);
	test_coupling(source_dests, dests.size(), count, bfs, pami_transport);
    }

    for (int i = 1; i <= 6; i++)
    {
        ratio = pow(2, i);

	if (rank == 0) {
	    printf ("\nSubset - Randomly distributed - Source:Dest ratio : %d : 1 c1\n", ratio);
	}

	subset_rdistributed(bfs.num_nodes, sources, dests, source_dests, ratio);
	test_coupling(source_dests, dests.size(), count, bfs, pami_transport);
    }

    for (int i = 1; i <= 3; i += 2)
    {
        if (rank == 0) {
            printf ("\nOverlapped - Contiguous - Source:Dest ratio : %d : 1 c1\n", i);
        }

        overlap_contiguous(bfs.num_nodes, sources, dests, source_dests, i);
        test_coupling(source_dests, dests.size(), count, bfs, pami_transport);
    }
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

    int count = 2 * 1024 * 1024;

    if (argc > 1) {
	count = atoi (argv[1]) * 1024;
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *) calloc (1, sizeof(struct optiq_pami_transport));

    optiq_pami_init_extra (pami_transport);
    optiq_pami_init (pami_transport);

    graphFilePath = "graph";

    if (world_rank == 0) {
	int cost = 1;
        optiq_graph_print_graph (bfs, cost, graphFilePath);
    }

    sleep(10);

    MPI_Barrier (MPI_COMM_WORLD);

    test_patterns (count, bfs, pami_transport);

    MPI_Finalize();

    return 0;
}
