#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "../topology_interface.h"

extern struct topology_interface topology_bgq;

void optiq_topology_init_bgq(struct topology_info *topo_info);
void optiq_topology_get_rank_bgq(struct topology_info *topo_info, int *rank);
void optiq_topology_get_num_ranks_bgq(struct topology_info *topo_info, int *num_ranks);
void optiq_topology_get_coord_bgq(struct topology_info *topo_info, int *coord);
void optiq_topology_get_physical_location_bgq(struct topology_info *topo_info, int *coord, physical_location *pl);
void optiq_topology_get_all_coords_bgq(struct topology_info *topo_info, int **all_coords);
void optiq_topology_get_all_node_ids_bgq(struct topology_info *topo_info, int *all_node_ids);
void optiq_topology_get_size_bgq(struct topology_info *topo_info, int *size);
void optiq_topology_get_torus_bgq(struct topology_info *topo_info, int *torus);
void optiq_topology_get_bridge_bgq(struct topology_info *topo_info, int *bridge_coord, int *bridge_id);
void optiq_topology_get_node_id_bgq(struct topology_info *topo_info, int *node_id);
void optiq_topology_get_node_id_from_coord_bgq(struct topology_info *topo_info, int *coord, int *node_id);
void optiq_topology_get_node_id_from_coord_bgq(struct topology_info *topo_info, int *coord, int *node_id);
void optiq_topology_get_neighbors_bgq(struct topology_info *topo_info, int *coord, optiq_neighbor *neighbors, int *num_neighbors);
void optiq_topology_get_topology_from_file_bgq(struct topology_info *topo_info, char *filePath);
void optiq_topology_get_topology_at_runtime_bgq(struct topology_info *topo_info);
void optiq_topology_get_node_bgq(struct topology_info *topo_info, struct optiq_node *node);
void optiq_topology_move_along_one_dimension_bgq(struct topology_info *topo_info, int *source, int routing_dimension, int num_hops, int direction, int **path);
void optiq_topology_reconstruct_path_bgq(struct topology_info *topo_info, int *source, int *dest, int **path);
void optiq_topology_compute_routing_order_bgq(struct topology_info *topo_info, int *order);
void optiq_topology_print_arcs_bgq(struct topology_info *topo_info, double cap);
void optiq_topology_finalize_bgq(struct topology_info *topo_info);

void optiq_topology_print_arcs_bgq(struct topology_info *topo, double cap);
void optiq_topology_compute_routing_order_bgq(struct topology_info *topo, int *order);
void optiq_topology_reconstruct_path_bgq(struct topology_info *topo, int *source, int *dest, int **path);
void optiq_topology_move_along_one_dimension_bgq(struct topology_info *topo, int *source, int routing_dimension, int num_hops, int direction, int **path);

#endif
