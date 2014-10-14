#ifndef TOPOLOGY_INTERFACE_H
#define TOPOLOGY_INTERFACE_H

#include "../utils/util.h"

enum machine_type {
    BGQ = 1,
    XE6 = 2,
    XC30 = 3
};

struct topology_info {
    int num_ranks;
    int num_dims;
    int *size;
    int *routing_order;
    int **all_coords;
    uint16_t *all_nic_ids;
};

struct topology_interface {
    enum machine_type machine;

    void (*optiq_topology_init)(struct topology_info *topo);
    void (*optiq_topology_get_rank)(struct topology_info *topo, int *rank);
    void (*optiq_topology_get_num_ranks)(struct topology_info *topo, int *num_ranks);
    void (*optiq_topology_get_nic_id)(struct topology_info *topo, uint16_t *nic_id);
    void (*optiq_topology_get_coord)(struct topology_info *topo, int *coord);
    void (*optiq_topology_get_all_coords)(struct topology_info *topo, int **all_coords);
    void (*optiq_topology_get_all_nic_ids)(struct topology_info *topo, uint16_t *all_nic_ids);
    void (*optiq_topology_get_size)(struct topology_info *topo, int *size);
    void (*optiq_topology_get_torus)(struct topology_info *topo, int *torus);
    void (*optiq_topology_get_bridge)(struct topology_info *topo, int *bridge_coord, int *bridge_id);
    void (*optiq_topology_get_node_id)(struct topology_info *topo, int *coord, int *node_id);
    void (*optiq_topology_get_neighbors)(struct topology_info *topo, int *coord, struct optiq_neighbor *neighbors, int num_neighbors);
    void (*optiq_topology_get_topology_at_runtime)(struct topology_info *topo);
    void (*optiq_topology_get_topology_from_file)(struct topology_info *topo, char *fileName);
    void (*optiq_topology_get_node)(struct topology_info *topo, struct optiq_node *node);
    void (*optiq_topology_finalize)(struct topology_info *topo);
};

#endif
