#ifndef OPTIQ_UTIL
#define OPTIQ_UTIL

#include "../system/topology_interface.h"

int optiq_compute_num_hops(int num_dims, int *source, int *dest);
void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims);
int optiq_check_existing(int num_elements, int *list, int element);
int optiq_check_existing_neighbor(int num_neighbors, optiq_neighbor *neighbors, int nid);

#endif
