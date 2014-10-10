#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#ifdef _CRAYC
#include <pmi.h>
#include <rca_lib.h>
#endif

#include <stdint.h>

struct topology {
    int rank;
    int num_ranks;
    uint16_t nic_id;
    int *coord;
    int num_dims;
    int *size;
    int *routing_order;
    int **all_coords;
    int *all_nic_ids;
};

struct optiq_node {
    int rank;
    int nic_id;
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

void optiq_read_topology_from_file(char *fileName, struct topology *topo);

int optiq_compute_nid(int num_dims, int *coord, int *size);
void optiq_coord_to_nodeId(int num_dims, int *size, int *coord, int *nodeId);
void optiq_move_along_one_dimension(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);
void optiq_reconstruct_path(int num_dims, int *size, int *source, int *dest, int *order, int *torus, int **path);
int optiq_compute_num_hops(int num_dims, int *source, int *dest);
void optiq_compute_routing_order(int num_dims, int *size, int *order);

#ifdef __bgq__
void optiq_map_ranks_to_coords(BG_CoordinateMapping_t *all_coord, int nranks);
#endif

int optiq_check_existing(int num_neighbors, int *neighbors, int nid);

int optiq_compute_neighbors_bgq(int num_dims, int *coord, int *size, int *neighbors);
void optiq_compute_neighbors_cray(int num_dims, int *coord, int **all_coords, int all_ranks, struct optiq_neighbor *neighbors, int num_neighbors);

void optiq_init();
void optiq_finalize();
void optiq_get_coord(int *coord);
void optiq_get_nic_id(uint16_t *nic_id);
void optiq_get_size(int *size);
void optiq_get_torus(int *torus);
void optiq_get_bridge(int *bridge_coord, int *bridge_id);
void optiq_get_all_coords(int **all_coords, int num_ranks);
void optiq_get_all_nic_ids(int *all_nic_ids, int num_ranks);
void optiq_get_topology(struct topology *topo);

void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims);
#endif
