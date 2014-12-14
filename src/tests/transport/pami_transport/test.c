#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "pami_transport.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int source = 0, dest = 1;

    if (world_rank == source) {
        /*Notify the size, ask for mem region*/


        /*Actual rput data*/  


        /*Notify that rput is done*/


    }

    if (world_rank == dest) {
        /*Return a mem region*/


        /*Wait for everything is done*/

    }

    return 0;
}
