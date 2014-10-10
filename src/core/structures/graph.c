#include <stdlib.h>
#include <stdio.h>

#include <topology.h>

#include <graph.h>

void construct_graph(struct topology topo, float **graph) 
{
    struct optiq_neighbor *neighbors = (struct optiq_neighbor *)malloc(sizeof(struct optiq_neighbor) * topo.num_dims * 2);

    int num_neighbors = 0;

    for (int i = 0; i < topo.num_ranks; i++) {
	optiq_compute_neighbors_cray(topo.num_dims, topo.all_coords[i], topo.all_coords, topo.num_ranks, neighbors, num_neighbors);

	num_neighbors = 0;
	for (int j = 0; j < topo.num_dims * 2; j++) {
	    if (neighbors[j].node.coord[0] != -1) {
		printf("Rank %d [ %d %d %d ] has neighbor [ %d %d %d ]\n", i, topo.all_coords[i][0], topo.all_coords[i][1], topo.all_coords[i][2], neighbors[j].node.coord[0], neighbors[j].node.coord[1], neighbors[j].node.coord[2]);
		num_neighbors++;
	    }
	}

	if (num_neighbors == 0) {
	    printf("Rank %d [ %d %d %d ] is isolated\n", i, topo.all_coords[i][0], topo.all_coords[i][1], topo.all_coords[i][2]);
	}

	printf("\n");
    }
}
