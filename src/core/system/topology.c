#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "topology.h"

void optiq_topology_init(struct topology *self, enum machine_type machine)
{
    if (machine == BGQ) {
	self->num_dims = 5;
	self->topo_impl = &topology_bgq;
    } else if (machine == XE6) {
	self->num_dims = 3;
	self->topo_impl = &topology_xe6;
    } else if (machine == XC30) {
	self->num_dims = 3;
	self->topo_impl = &topology_xc30;
    } else {
	/*self->topo_impl = &topology_user_defined;*/
    }
    self->topo_impl->optiq_topology_init(self);
}

void optiq_topology_get_rank(struct topology *self, int *rank)
{
    self->topo_impl->optiq_topology_get_rank(self, rank);
}

void optiq_topology_get_num_ranks(struct topology *self, int *num_ranks)
{
    self->topo_impl->optiq_topology_get_num_ranks(self, num_ranks);
}

void optiq_topology_get_nic_id(struct topology *self, uint16_t *nid)
{
    self->topo_impl->optiq_topology_get_nic_id(self, nid);
}

void optiq_topology_get_coord(struct topology *self, int *coord)
{
    self->topo_impl->optiq_topology_get_coord(self, coord);
}

void optiq_topology_get_all_coords(struct topology *self, int **all_coords)               
{
    self->topo_impl->optiq_topology_get_all_coords(self, all_coords);
}

void optiq_topology_get_all_nic_ids(struct topology *self, uint16_t *all_nic_ids)
{
    self->topo_impl->optiq_topology_get_all_nic_ids(self, all_nic_ids);
}

void optiq_topology_get_size(struct topology *self, int *size)
{
    self->topo_impl->optiq_topology_get_size(self, size);
}

void optiq_topology_get_torus(struct topology *self, int *torus)
{
    self->topo_impl->optiq_topology_get_torus(self, torus);
}

void optiq_topology_get_bridge(struct topology *self, int *bridge_coord, int *brige_id)
{
    self->topo_impl->optiq_topology_get_bridge(self, bridge_coord, brige_id);
}

void optiq_topology_get_node_id(struct topology *self, int *coord, int *node_id)
{
    self->topo_impl->optiq_topology_get_node_id(self, coord, node_id);
}

void optiq_topology_get_neighbors(struct topology *self, int *coord, struct optiq_neighbor *neighbors, int num_neighbors) 
{
    self->topo_impl->optiq_topology_get_neighbors(self, coord, neighbors, num_neighbors);
}

void optiq_topology_get_topology_at_runtime(struct topology *self)
{
    self->topo_impl->optiq_topology_get_topology_at_runtime(self);
}

void optiq_topology_get_topology_from_file(struct topology *self, char *fileName)
{
    self->topo_impl->optiq_topology_get_topology_from_file(self, fileName);
}

void optiq_topology_get_node(struct topology *self, struct optiq_node *node)
{
    self->topo_impl->optiq_topology_get_node(self, node);
}

void optiq_topology_finalize(struct topology *self) 
{

}
