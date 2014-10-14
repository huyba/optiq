#ifndef OPTIQ_TOPOLOGY_H
#define OPTIQ_TOPOLOGY_H

#include <stdint.h>

#include "topology_interface.h"

#include "topology_bgq.h"
#include "topology_xe6.h"
#include "topology_xc30.h"

struct topology {
    int num_ranks;
    int num_dims;
    int *size;
    int *routing_order;
    int **all_coords;
    uint16_t *all_nic_ids;

    struct topology_interface *topo_impl;
};

struct optiq_node {
    int rank;
    uint16_t nic_id;
    int *coord;
};

enum optiq_direction {
    A_P = 1,
    A_M = -1,
    B_P = 2,
    B_M = -2,
    C_P = 3,
    C_M = -3,
    D_P = 4,
    D_M = -4,
    E_P = 5,
    E_M = -5,
    X_P = 1,
    X_M = -1,
    Y_P = 2,
    Y_M = -2,
    Z_P = 3,
    Z_M = -3,
    MIXED = 1000
};

struct optiq_neighbor {
    struct optiq_node node;
    int distance;
    float link_capacity;
    enum optiq_direction direction;
};

void optiq_topology_init(struct topology *self);
void optiq_topology_get_rank(struct topology *self, int *rank);
void optiq_topology_get_num_ranks(struct topology *self, int *num_ranks);
void optiq_topology_get_nic_id(struct topology *self, uint16_t *nic_id);
void optiq_topology_get_coord(struct topology *self, int *coord);
void optiq_topology_get_all_coords(struct topology *self, int **all_coords);
void optiq_topology_get_all_nic_ids(struct topology *self, uint16_t *all_nic_ids);
void optiq_topology_get_size(struct topology *self, int *size);
void optiq_topology_get_torus(struct topology *self, int *torus);
void optiq_topology_get_bridge(struct topology *self, int *bridge_coord, int *bridge_id);
void optiq_topology_get_node_id(struct topology *self, int *coord, int *node_id);
void optiq_topology_get_neighbors(struct topology *self, int *coord, struct optiq_neighbor *neighbors, int num_neighbors);
void optiq_topology_get_topology_at_runtime(struct topology *self);
void optiq_topology_get_topology_from_file(struct topology *self, char *fileName);
void optiq_topology_get_node(struct topology *self, struct optiq_node *node);
void optiq_topology_finalize(struct topology *self);

#endif
