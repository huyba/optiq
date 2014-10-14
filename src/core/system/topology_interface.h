#ifndef TOPOLOGY_IMPL_H
#define TOPOLOGY_IMPL_H

#include "../util/util.h"

enum machine_type {
    BGQ = 1,
    XE6 = 2,
    XC30 = 3
};

struct topology_interface {
    enum machine_type machine;

    void (*optiq_topology_init)(struct topology *self, enum machine_type machine);
    void (*optiq_topology_finalize)(struct topology *self);
    void (*optiq_topology_get_coord)(struct topology *self, int *coord);
    void (*optiq_topology_get_nic_id)(struct topology *self, uint16_t *nic_id);
    void (*optiq_topology_get_size)(struct topology *self, int *size);
    void (*optiq_topology_get_torus)(struct topology *self, int *torus);
    void (*optiq_topology_get_bridge)(truct topology *self, nt *bridge_coord, int *bridge_id);
    void (*optiq_topology_map_ranks_coords)(struct topology *self, int **all_coords, int num_ranks);
    void (*optiq_topology_get_all_nic_ids)(struct topology *self, uint16_t *all_nic_ids, int num_ranks);
    void (*optiq_topology_get_topology_at_runtime)(struct topology *self);
    void (*optiq_topology_get_node)(struct topology *self, struct optiq_node *node, int num_dims);

    void optiq_topology_get_node_id(struct topology *self, , int *coord, int *node_id);
    void optiq_coord_to_nodeId(struct topology *self, int *coord, int *nodeId);
    void optiq_topology_reconstruct_path(struct topology *self, int *source, int *dest, int **path);
    void optiq_toplogy_compute_routing_order(struct topology *self, int *order);
    int optiq_compute_neighbors(struct topology *self, struct optiq_neighbor *neighbors, int num_neighbors);

    void optiq_move_along_one_dimension(struct topology *self, int *source, int routing_dimension, int num_hops, int dir
ection, int **path);

    void optiq_get_topology_from_file(struct topology *self, har *fileName);

    int optiq_compute_num_hops(int num_dims, int *source, int *dest);
    int optiq_check_existing(int num_neighbors, int *neighbors, int nid);
    void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims);
}

#endif
