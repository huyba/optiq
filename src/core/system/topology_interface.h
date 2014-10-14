#ifndef TOPOLOGY_IMPL_H
#define TOPOLOGY_IMPL_H

#include "../utils/util.h"

enum machine_type {
    BGQ = 1,
    XE6 = 2,
    XC30 = 3
};

struct topology_interface {
    enum machine_type machine;

    void (*optiq_topology_init)(struct topology *self, enum machine_type machine);
    void (*optiq_topology_get_rank)(struct topology *self, int *rank);
    void (*optiq_topology_get_num_ranks)(struct topology *self, int *num_ranks);
    void (*optiq_topology_get_nic_id)(struct topology *self, uint16_t *nic_id);
    void (*optiq_topology_get_coord)(struct topology *self, int *coord);
    void (*optiq_topology_get_all_coords)(struct topology *self, int **all_coords);
    void (*optiq_topology_get_all_nic_ids)(struct topology *self, uint16_t *all_nic_ids);
    void (*optiq_topology_get_size)(struct topology *self, int *size);
    void (*optiq_topology_get_torus)(struct topology *self, int *torus);
    void (*optiq_topology_get_bridge)(struct topology *self, int *bridge_coord, int *bridge_id);
    void (*optiq_topology_get_node_id)(struct topology *self, int *coord, int *node_id);
    void (*optiq_topology_compute_neighbors)(struct topology *self, int *coord, struct optiq_neighbor *neighbors, int num_neighbors);
    void (*optiq_topology_get_topology_at_runtime)(struct topology *self);
    void (*optiq_topology_get_topology_from_file)(struct topology *self, char *fileName);
    void (*optiq_topology_get_node)(struct topology *self, struct optiq_node *node);
    void (*optiq_topology_finalize)(struct topology *self);
}

#endif
