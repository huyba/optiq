#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

int optiq_compute_nid(int num_dims, int *size, int *coord);

int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors);

void optiq_topology_compute_routing_order_bgq(int num_dims, int *size, int *order);

void optiq_topology_move_along_one_dimension_bgq(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);

void optiq_topology_reconstruct_path_bgq(int num_dims, int *size, int *torus, int *order, int *source, int *dest, int **path);

#endif
