#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef _CRAYC

#include "graph_xe6.h"

void optiq_graph_init_xe6(struct topology_info *topo_info)
{

}

void optiq_graph_construct_xe6(struct topology_info *topo_info, float **graph)
{
    printf("Start constructing graph\n");

    struct optiq_neighbor *neighbors = (struct optiq_neighbor *)malloc(sizeof(struct optiq_neighbor) * topo.num_dims * 2);

    for (int i = 0; i < topo.num_dims * 2; i++) {
        neighbors[i].node.coord = (int *)malloc(sizeof(int) * topo.num_dims);
    }

    printf("Init the neighbors\n");

    int num_neighbors = 0;

    for (int i = 0; i < topo.num_ranks; i++) {
        optiq_compute_neighbors_cray(topo.num_dims, topo.all_coords[i], topo.all_coords, topo.num_ranks, neighbors);

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
}

#endif
