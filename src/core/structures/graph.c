#include <stdlib.h>
#include <stdio.h>

#include <graph.h>

#include <topology.h>

void construct_graph(struct topology topo) 
{
    int **neighbors_coords;

    neighbors_coords = (int **)malloc(sizeof(int *) * topo.num_dims * 2);
    for (int i = 0; i < topo.num_dims * 2; i++) {
	neighbors_coords[i] = (int *)malloc(sizeof(int) * topo.num_dims);
    }

    for (int i = 0; i < topo.num_ranks; i++) {
	optiq_compute_neighbors_cray(topo.num_dims, topo.all_coords[i], topo.all_coords, topo.num_ranks, neighbors_coords);

	for (int j = 0; j < topo.num_dims * 2; j++) {
	    if (neighbors_coords[j][0] != -1) {
		printf("Neighbor coord[ %d %d %d ]\n", neighbors_coords[j][0], neighbors_coords[j][1], neighbors_coords[j][2]);
	    }
	}
    }
}
