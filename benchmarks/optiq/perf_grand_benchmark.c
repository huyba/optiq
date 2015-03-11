#include "optiq.h"
#include "mpi_benchmark.h"

#include <mpi.h>

void benchmark_for_a_pattern (char *filepath, int rank, int size)
{
    void *sendbuf = NULL, *recvbuf = NULL;
    int *sendcounts, *sdispls, *recvcounts, *rdispls;

    optiq_patterns_alltoallv_from_file (filepath, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, rank, size);

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    std::vector<struct path *> mpi_paths;
    optiq_benchmark_reconstruct_mpi_paths(sendcounts, mpi_paths);

    if (rank == 0) {
	optiq_path_print_stat(mpi_paths, size);
    }

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect(rank);

    if (rank == 0) {
        optiq_path_print_stat(schedule->paths, size);
    }

    for (int i = 0; i < schedule->paths.size(); i++) {
        free(schedule->paths[i]);
    }
    schedule->paths.clear();
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

    /* Benchmark for first k last k pattern */
    for (int i = 2; i <= 8; i  *= 2) 
    {
	if (rank == 0) 
	{
	    printf("First %d nodes send data to last %d nodes\n", size/i);
	    optiq_pattern_firstk_lastk(filepath, size, demand, size/i);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	benchmark_for_a_pattern(filepath, rank, size);
    }
    
    if (rank == 0) {
        printf("Finished benchmarking\n");
    }

    optiq_finalize();

    return 0;
}
