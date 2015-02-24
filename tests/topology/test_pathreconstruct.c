#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <utility>

#include <mpi.h>

#include "patterns.h"
#include "topology.h"
#include "pathreconstruct.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int num_nodes = world_size;
    std::vector<int> sources;
    std::vector<int> dests;
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int ratio = 1;

    struct topology topo;

    optiq_topology_init(&topo);

    std::vector<struct path *> mpi_paths;


    disjoint_contiguous(num_nodes, sources, dests, source_dests, ratio);
    mpi_paths.clear();
    reconstruct_paths (source_dests, topo, mpi_paths);

    if (world_rank == 0) {
	optiq_path_print_stat(mpi_paths, num_nodes);
    }

    int k = num_nodes/4;
    disjoint_contiguous_firstk_lastk(num_nodes, sources, dests, source_dests, k);
    mpi_paths.clear();
    reconstruct_paths (source_dests, topo, mpi_paths);

    if (world_rank == 0) {
        optiq_path_print_stat(mpi_paths, num_nodes);
    }

    return 0;
}
