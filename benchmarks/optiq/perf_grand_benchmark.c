#include "optiq.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

#include <mpi.h>

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

    /* First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    if (rank == 0)
	    {
		printf("First %d nodes send data to last %d nodes\n", m, n);
		optiq_pattern_firstm_lastn(filepath, size, demand, m, n);
	    }

	    MPI_Barrier(MPI_COMM_WORLD);
	    optiq_benchmark_pattern_from_file (filepath, rank, size);
	}
    }

    /* Benchmark for subgroup aggregation */
    for (int i = 4; i < 128; i *= 2)
    {
        if (rank == 0)
        {
            printf("Subgroup of %d nodes aggregate data to node in the middle of the subgroup\n", i);
	    optiq_pattern_subgroup_agg(filepath, size, i, demand);
        }
        MPI_Barrier(MPI_COMM_WORLD);

        optiq_benchmark_pattern_from_file (filepath, rank, size);
    }
    
    if (rank == 0) {
        printf("Finished benchmarking\n");
    }

    optiq_finalize();

    return 0;
}
