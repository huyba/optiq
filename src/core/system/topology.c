#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "topology.h"

void optiq_topology_init(struct optiq_topology *self, enum machine_type machine)
{
    self->topo_info = (struct topology_info*)malloc(sizeof(struct topology_info));

    if (machine == BGQ) {
	self->topo_info->num_dims = 5;
	self->topo_impl = &topology_bgq;
    } else if (machine == XE6) {
	self->topo_info->num_dims = 3;
	self->topo_impl = &topology_xe6;
    } else if (machine == XC30) {
	self->topo_info->num_dims = 3;
	self->topo_impl = &topology_xc30;
    } else {
	/*self->topo_impl = &topology_user_defined;*/
    }
    self->topo_impl->optiq_topology_init(self->topo_info);
}

void optiq_topology_get_rank(struct optiq_topology *self, int *rank)
{
    self->topo_impl->optiq_topology_get_rank(self->topo_info, rank);
}

void optiq_topology_get_num_ranks(struct optiq_topology *self, int *num_ranks)
{
    self->topo_impl->optiq_topology_get_num_ranks(self->topo_info, num_ranks);
}

void optiq_topology_get_node_id(struct optiq_topology *self, int *nid)
{
    self->topo_impl->optiq_topology_get_node_id(self->topo_info, nid);
}

void optiq_topology_get_node_id_from_coord(struct optiq_topology *self, int *coord, int *nid)
{
    self->topo_impl->optiq_topology_get_node_id_from_coord(self->topo_info, coord, nid);
}

void optiq_topology_get_coord(struct optiq_topology *self, int *coord)
{
    self->topo_impl->optiq_topology_get_coord(self->topo_info, coord);
}

void optiq_topology_get_physical_location(struct optiq_topology *self, int *coord, physical_location *pl)
{
    self->topo_impl->optiq_topology_get_physical_location(self->topo_info, coord, pl);
}

void optiq_topology_get_all_coords(struct optiq_topology *self, int **all_coords)               
{
    self->topo_impl->optiq_topology_get_all_coords(self->topo_info, all_coords);
}

void optiq_topology_get_all_node_ids(struct optiq_topology *self, int *all_node_ids)
{
    self->topo_impl->optiq_topology_get_all_node_ids(self->topo_info, all_node_ids);
}

void optiq_topology_get_size(struct optiq_topology *self, int *size)
{
    self->topo_impl->optiq_topology_get_size(self->topo_info, size);
}

void optiq_topology_get_torus(struct optiq_topology *self, int *torus)
{
    self->topo_impl->optiq_topology_get_torus(self->topo_info, torus);
}

void optiq_topology_get_bridge(struct optiq_topology *self, int *bridge_coord, int *brige_id)
{
    self->topo_impl->optiq_topology_get_bridge(self->topo_info, bridge_coord, brige_id);
}

void optiq_topology_get_neighbors(struct optiq_topology *self, int *coord, struct optiq_neighbor *neighbors, int *num_neighbors) 
{
    self->topo_impl->optiq_topology_get_neighbors(self->topo_info, coord, neighbors, num_neighbors);
}

void optiq_topology_get_topology_at_runtime(struct optiq_topology *self)
{
    self->topo_impl->optiq_topology_get_topology_at_runtime(self->topo_info);
}

void optiq_topology_get_topology_from_file(struct optiq_topology *self, char *fileName)
{
    self->topo_impl->optiq_topology_get_topology_from_file(self->topo_info, fileName);
}

void optiq_topology_get_node(struct optiq_topology *self, struct optiq_node *node)
{
    self->topo_impl->optiq_topology_get_node(self->topo_info, node);
}

void optiq_topology_finalize(struct optiq_topology *self) 
{
    self->topo_impl->optiq_topology_finalize(self->topo_info);
}
