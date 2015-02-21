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

    struct topology topo;
    optiq_topology_init(&topo);

    optiq_topology_print(topo);

    return 0;
}
