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

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    optiq_topology_init();
    struct topology *tp = optiq_topology_get();

    if (world_rank == 0) {
	optiq_topology_print(tp);
    }

    return 0;
}
