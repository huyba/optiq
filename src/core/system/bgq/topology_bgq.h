#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "../topology_interface.h"

extern struct topology_interface topology_bgq;

void optiq_topology_print_arcs_bgq(struct topology_info *topo, double cap);
void optiq_topology_compute_routing_order_bgq(struct topology_info *topo, int *order);
void optiq_topology_reconstruct_path_bgq(struct topology_info *topo, int *source, int *dest, int **path);
void optiq_topology_move_along_one_dimension_bgq(struct topology_info *topo, int *source, int routing_dimension, int num_hops, int direction, int **path);

#endif
