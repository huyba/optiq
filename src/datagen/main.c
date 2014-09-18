#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "topology.h"

int main(int argc, char **argv) 
{
    MPI_Init(&argc, &argv);

    int world_rank, world_size;
 
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int coord[5], size[5], bridge[5], bridgeId;

    getTopology(coord, size, bridge, &bridgeId);

    if (world_rank == 0) {
	fprintf(stderr, "Torus dimension %d x %d x %d x %d x %d\n", size[0], size[1], size[2], size[3], size[4]);
    }

    fprintf(stderr, "Rank %d: [%d, %d, %d, %d, %d], bridge  %d: [%d, %d, %d, %d, %d]\n", world_rank, coord[0], coord[1], coord[2], coord[3], coord[4], bridgeId, bridge[0], bridge[1], bridge[2], bridge[3], bridge[4]);

    int num_dims = 5;
    int num_sources = atoi(argv[1]);
    int factor = 1;

    int *allBridges = (int *) malloc(sizeof(int) * world_size);
    MPI_Allgather(&bridgeId, 1, MPI_INT, allBridges, 1, MPI_INT, MPI_COMM_WORLD);

    int num_bridges = world_size/128*2;
    int *bridges = (int *)malloc(sizeof(int) * num_bridges);
    bridges[0] = allBridges[0];
    int i = 0, j = 1;
    for (i = 1; i < world_size; i++) {
	if (!check_existing(j, bridges, allBridges[i])) {
	    bridges[j] = allBridges[i];
	    j++;
	}
    } 
    
    if (world_rank == 0) {
	generateDataIO(num_dims, size, num_sources, factor, num_bridges, bridges);
    }

    MPI_Finalize();
}
