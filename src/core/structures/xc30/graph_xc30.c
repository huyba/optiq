#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "graph_xc30.h"

void optiq_graph_init_xc30(struct topology_info *topo_info)
{

}

void optiq_graph_construct_xc30(struct topology_info *topo_info, optiq_graph **graph)
{

}

void optiq_graph_coarsen_xc30(struct optiq_graph *graph)
{

}

void optiq_graph_uncoarsen_xc30(struct optiq_graph *graph)
{

}

struct graph_interface graph_xc30 =
{
    .optiq_graph_init = optiq_graph_init_xc30,
    .optiq_graph_construct = optiq_graph_construct_xc30,
    .optiq_graph_coarsen = optiq_graph_coarsen_xc30,
    .optiq_graph_uncoarsen = optiq_graph_uncoarsen_xc30
}
