#ifndef OPTIQ_GRAPH_H
#define OPTIQ_GRAPH_H

#include <stdint.h>

#include "../system/topology.h"
#include "graph_interface.h"

#include "graph_bgq.h"
#include "graph_xe6.h"
#include "graph_xc30.h"

struct graph {
    struct topology_info *topo_info;
    struct graph_interface *graph_impl;
};

void optiq_graph_init(struct graph *self);
void optiq_graph_construct(struct graph *self, float **graph);
void optiq_graph_coarsen(struct optiq_graph *graph);
void optiq_graph_uncoarsen(struct optiq_graph *graph);

#endif
