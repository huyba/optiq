#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

#include<vector>

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

struct topology {
    int num_dims;
    int *size;
    int num_nodes;
    int num_edges;

    int **coords;
    std::vector<int> *neighbors;
};

void optiq_topology_init();

void optiq_topology_get_size_bgq(int *size);

int optiq_topology_compute_node_id(int num_dims, int *size, int *coord);

int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors);

std::vector<int> * optiq_topology_get_all_nodes_neighbors(int num_dims, int *size);

void optiq_topology_compute_routing_order_bgq(int num_dims, int *size, int *order);

void optiq_topology_move_along_one_dimension_bgq(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);

void optiq_topology_reconstruct_path_bgq(int num_dims, int *size, int *torus, int *order, int *source, int *dest, int **path);

#endif
