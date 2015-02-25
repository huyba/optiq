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
    int num_nodes;
    int num_edges;

    int size[5];
    int torus[5];
    int order[5];

    int coord[5];
    int **all_coords;
    std::vector<int> *neighbors;

    int world_rank;
    int world_size;
    int num_ranks_per_node;
    bool initialized;
    bool finalized;
};

extern "C" struct topology *topo;

void optiq_topology_init();

void optiq_topology_init_with_params(int num_dims, int *size, struct topology *topo);

struct topology* optiq_topology_get();

void optiq_topology_print(struct topology *topo);

void optiq_topology_get_size_bgq(int *size);

int optiq_topology_get_node_id(int world_rank, int num_ranks_per_node);

int optiq_topology_get_coord(int *coord);

int optiq_topology_compute_node_id(int num_dims, int *size, int *coord);

int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors);

std::vector<int> * optiq_topology_get_all_nodes_neighbors(int num_dims, int *size);

int** optiq_topology_get_all_coords (int num_dims, int *size);

void optiq_topology_get_torus(int *torus);

void optiq_topology_compute_routing_order_bgq(int num_dims, int *size, int *order);

void optiq_topology_move_along_one_dimension_bgq(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);

void optiq_topology_reconstruct_path_bgq(int num_dims, int *size, int *torus, int *order, int *source, int *dest, int **path);

void optiq_topology_print_all_arcs(int num_dims, int *size, double cap);

void optiq_topology_finalize();

#endif
