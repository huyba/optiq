#ifndef OPTIQ_TOPOLOGY_H
#define OPTIQ_TOPOLOGY_H

#include <stdint.h>

#include "topology_interface.h"

#include "bgq/topology_bgq.h"
#include "xe6/topology_xe6.h"
#include "xc30/topology_xc30.h"

struct topology {
    struct topology_info *topo_info;
    struct topology_interface *topo_impl;
};

void optiq_topology_init(struct topology *self, machine_type machine);
void optiq_topology_get_rank(struct topology *self, int *rank);
void optiq_topology_get_num_ranks(struct topology *self, int *num_ranks);
void optiq_topology_get_coord(struct topology *self, int *coord);
void optiq_topology_get_node_id(struct topology *self, int *node_id);
void optiq_topology_get_physical_location(struct topology *self, int *coord, physical_location *pl);
void optiq_topology_get_all_coords(struct topology *self, int **all_coords);
void optiq_topology_get_all_node_ids(struct topology *self, int *all_node_ids);
void optiq_topology_get_size(struct topology *self, int *size);
void optiq_topology_get_torus(struct topology *self, int *torus);
void optiq_topology_get_bridge(struct topology *self, int *bridge_coord, int *bridge_id);
void optiq_topology_get_node_id_from_coord(struct topology *self, int *coord, int *node_id);
void optiq_topology_get_neighbors(struct topology *self, int *coord, struct optiq_neighbor *neighbors, int *num_neighbors);
void optiq_topology_get_topology_at_runtime(struct topology *self);
void optiq_topology_get_topology_from_file(struct topology *self, char *fileName);
void optiq_topology_get_node(struct topology *self, struct optiq_node *node);
void optiq_topology_finalize(struct topology *self);

#endif
