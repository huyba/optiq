#ifndef TOPOLOGY_H
#define TOPOLOGY_H

struct topology {
    int num_ranks;
    int num_dims;
    int *size;
    int *routing_order;
    int **all_coords;
    uint16_t *all_nic_ids;

    struct topology_inteface *topo_impl;
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
#endif
