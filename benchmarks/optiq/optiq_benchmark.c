#include "optiq.h"
#include "opi.h"
#include "mpi_benchmark.h"

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size)
{
    void *sendbuf = NULL, *recvbuf = NULL;
    int *sendcounts, *sdispls, *recvcounts, *rdispls;

    struct topology *topo = optiq_topology_get();

    optiq_patterns_alltoallv_from_file (filepath, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, rank, size);

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    std::vector<struct path *> mpi_paths;
    optiq_benchmark_reconstruct_mpi_paths(sendcounts, mpi_paths);

    if (rank == 0) {
        optiq_path_print_stat(mpi_paths, size, topo->num_edges);
    }

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect(rank);

    if (rank == 0) {
        optiq_path_print_stat(opi.paths, size, topo->num_edges);
    }
}
