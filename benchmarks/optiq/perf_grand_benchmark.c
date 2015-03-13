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

    /* Benchmark for first k last k pattern */
    for (int i = 2; i <= 8; i  *= 2) 
    {
	if (rank == 0) 
	{
	    printf("First %d nodes send data to last %d nodes\n", size/i, size/i);
	    optiq_pattern_firstk_lastk(filepath, size, demand, size/i);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	optiq_benchmark_pattern_from_file (filepath, rank, size);
    }

    /* Benchmark for last k to first k pattern */
    for (int i = 2; i <= 8; i  *= 2)
    {
        if (rank == 0)
        {
            printf("Last %d nodes send data to first %d nodes\n", size/i, size/i);
            optiq_pattern_lastk_firstk(filepath, size, demand, size/i);
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
