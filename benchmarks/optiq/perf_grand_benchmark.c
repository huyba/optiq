#include "optiq.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

#include <mpi.h>

int testid = 1;

void test(int rank, int size, int demand, char *filepath)
{
    /* First m send data to last n */
    /*for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    if (rank == 0)
	    {
		printf("Test No. %d: First %d ranks send data to last %d ranks\n", testid, m, n);
		optiq_pattern_firstm_lastn(filepath, size, demand, m, n, false);
		testid++;
	    }

	    MPI_Barrier(MPI_COMM_WORLD);
	    optiq_benchmark_pattern_from_file (filepath, rank, size);

	    if (rank == 0)
            {
                printf("Test No. %d: First %d ranks send data to last %d randomize ranks\n", testid, m, n);
                optiq_pattern_firstm_lastn(filepath, size, demand, m, n, true);
                testid++;
            }

            MPI_Barrier(MPI_COMM_WORLD);
            optiq_benchmark_pattern_from_file (filepath, rank, size);
	}
    }*/

    /* Benchmark for subgroup aggregation */
    for (int i = 4; i < 128; i *= 2)
    {
        if (rank == 0)
        {
            printf("Test No. %d: Subgroup of %d ranks aggregate data to rank in the middle of the subgroup\n", testid, i * topo->num_ranks_per_node);
	    optiq_pattern_subgroup_agg(filepath, size, i * topo->num_ranks_per_node, demand);
	    testid++;
        }
        MPI_Barrier(MPI_COMM_WORLD);

        optiq_benchmark_pattern_from_file (filepath, rank, size);
    }

    /* Benchmark for overlapped groups */
    for (int m = size/16; m <= size/2; m *= 2)
    {
        for (int n = size/16; n <= size/2; n *= 2)
        {
	    for (int ov = 2; ov <= 4; ov *= 2) 
	    {
		if (rank == 0)
		{
		    printf("Test No. %d: First %d ranks send data to %d ranks, with %d ranks overlapped\n", testid, m, n, n/ov);
		    optiq_pattern_overlap (filepath, size, demand, m, m/ov < n/ov ? m/ov : n/ov , n, false);
		    testid++;
		}

		MPI_Barrier(MPI_COMM_WORLD);

		optiq_benchmark_pattern_from_file (filepath, rank, size);

		if (rank == 0)
                {
                    printf("Test No. %d: First %d ranks send data to %d ranks, with %d ranks overlapped, randomized\n", testid, m, n, n/ov);
                    optiq_pattern_overlap (filepath, size, demand, m, m/ov < n/ov ? m/ov : n/ov , n, true);
                    testid++;
                }

                MPI_Barrier(MPI_COMM_WORLD);

                optiq_benchmark_pattern_from_file (filepath, rank, size);
	    }
        }
    }
}

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    char *filepath = "pattern";
    int demand = 1024 * 1024;

    if (argc > 1) {
        filepath = argv[1];
    }
    if (argc > 2) {
        demand = atoi(argv[2]);
    }

    struct optiq_algorithm *alg = optiq_algorithm_get();
    alg->search_alg = OPTIQ_ALG_KPATHS;
    alg->num_paths_per_pair = 3;

    //odp.print_path_id = true;
    //odp.print_path_rank = true;
    //odp.print_local_jobs = true;
    //odp.print_sourcedests_id = true;
    //odp.print_sourcedests_rank = true;
    //odp.print_pami_transport_status = true;

    test(rank, size, demand, filepath);
    
    if (rank == 0) {
        printf("Finished benchmarking\n");
    }

    optiq_finalize();

    return 0;
}
