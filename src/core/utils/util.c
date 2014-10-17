#include "stdlib.h"
#include "util.h"

int optiq_compute_num_hops(int num_dims, int *source, int *dest)
{
    int num_hops = 0;
    for (int i = 0; i < num_dims; i++) {
	num_hops += abs(source[i] - dest[i]);
    }
    return num_hops;
}

void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims)
{
    int potential_distance = optiq_compute_num_hops(num_dims, coord, potential_neighbor.node.coord);

    if (potential_distance < current_neighbor->distance) {
	current_neighbor->distance = potential_distance;
	for (int i = 0; i < num_dims; i++) {
	    current_neighbor->node.coord[i] = potential_neighbor.node.coord[i];
	}
    }
}

int optiq_check_existing(int num_elements, int *list, int element)
{
    for (int i = 0; i < num_elements; i++) {
        if (list[i] == element) {
            return 1;
        }
    }

    return 0;
}

int optiq_check_existing_neighbor(int num_neighbors, optiq_neighbor *neighbors, int nid)
{
    for (int i = 0; i < num_neighbors; i++) {
        if (neighbors[i].node.rank == nid) {
            return 1;
        }
    }

    return 0;
}

