#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

void optiq_topology_get_node_id_from_coord_bgq(int num_dims, int *size, int *coord, int *node_id);

void optiq_topology_get_neighbor_ids_bgq(int num_dims, int *size, int *coord, int *neighbors, int *num_neighbors);

#endif
