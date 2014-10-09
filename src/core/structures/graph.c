#include <stdlib.h>
#include <stdio.h>

#include <topology.h>

#include <graph.h>

void construct_graph(struct topology topo, float **graph) 
{
    int **neighbors_coords;

    neighbors_coords = (int **)malloc(sizeof(int *) * topo.num_dims * 2);
    for (int i = 0; i < topo.num_dims * 2; i++) {
	neighbors_coords[i] = (int *)malloc(sizeof(int) * topo.num_dims);
    }

    int num_neighbors = 0;

    for (int i = 0; i < topo.num_ranks; i++) {
	optiq_compute_neighbors_cray(topo.num_dims, topo.all_coords[i], topo.all_coords, topo.num_ranks, neighbors_coords);

	num_neighbors = 0;
	for (int j = 0; j < topo.num_dims * 2; j++) {
	    if (neighbors_coords[j][0] != -1) {
		printf("Rank %d [ %d %d %d ] has neighbor [ %d %d %d ]\n", i, topo.all_coords[i][0], topo.all_coords[i][1], topo.all_coords[i][2], neighbors_coords[j][0], neighbors_coords[j][1], neighbors_coords[j][2]);
		num_neighbors++;
	    }
	}

	if (num_neighbors == 0) {
	    printf("Rank %d [ %d %d %d ] is isolated\n", i, topo.all_coords[i][0], topo.all_coords[i][1], topo.all_coords[i][2]);
	}

	printf("\n");
    }
}
