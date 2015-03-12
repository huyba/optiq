#include "optiq.h"
#include "opi.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths)
{
    mpi_paths.clear();

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    source_dests.clear();

    optiq_schedule_get_pair (sendcounts, source_dests);

    /*if (pami_transport->rank == 0) {
	optiq_schedule_print_sourcedests(source_dests);
    }*/

    struct topology *topo = optiq_topology_get();

    optiq_topology_path_reconstruct (source_dests, topo, mpi_paths);
}

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size)
{
    struct topology *topo = optiq_topology_get();

    void *sendbuf = NULL, *recvbuf = NULL;
    int *sendcounts, *sdispls, *recvcounts, *rdispls;

    optiq_patterns_alltoallv_from_file (filepath, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, rank, size);

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    std::vector<struct path *> mpi_paths;
    mpi_paths.clear();
    optiq_benchmark_reconstruct_mpi_paths(sendcounts, mpi_paths);

    if (rank == 0) {
	//optiq_path_print_paths_coords(mpi_paths, topo->all_coords);
        optiq_path_print_stat(mpi_paths, size, topo->num_edges);
    }

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect(rank);

    if (rank == 0) {
	//optiq_path_print_paths_coords(opi.paths, topo->all_coords);
        optiq_path_print_stat(opi.paths, size, topo->num_edges);
    }

    optiq_opi_clear();
}

