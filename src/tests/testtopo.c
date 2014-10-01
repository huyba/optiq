#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#ifndef __cray__
#define __cray__
#endif

#include "topology.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int num_dims = 3;
    int nid, coords[3];
    optiq_get_coordinates(coords, &nid);

    printf("Rank: %d nid %d coord[ %d %d %d ]\n", world_rank, nid, coords[0], coords[1], coords[2]);

    MPI_Finalize();
}
