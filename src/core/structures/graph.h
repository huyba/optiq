#ifndef OPTIQ_GRAPH_H
#define OPTIQ_GRAPH_H

#include <stdint.h>

#include "../system/topology.h"
#include "graph_interface.h"

#include "bgq/graph_bgq.h"
#include "xe6/graph_xe6.h"
#include "xc30/graph_xc30.h"

struct optiq_graph {
    struct topology_info *topo_info;
    struct optiq_graph_info *graph;
    struct optiq_graph_interface *graph_impl;
};

void optiq_graph_init(struct optiq_graph *self, machine_type machine);
void optiq_graph_construct(struct optiq_graph *self, struct optiq_graph_info *graph_info);
void optiq_graph_coarsen(struct optiq_graph *self, struct optiq_graph_info *graph_info);
void optiq_graph_uncoarsen(struct optiq_graph *self, struct optiq_graph_info *graph_info);

#endif
