#include <stdlib.h>
#include <stdio.h>

#include <topology.h>

#include <graph.h>

void optiq_graph_init(struct optiq_graph *self, machine_type machine) 
{
    machine_type machine = self->topo_info->machine;

    if (machine == BGQ) {
        self->graph_impl = &graph_bgq;
    } else if (machine == XE6) {
        self->graph_impl = &graph_xe6;
    } else if (machine == XC30) {
        self->graph_impl = &graph_xc30;
    } else {
        /*self->topo_impl = &topology_user_defined;*/
    }
    self->graph_impl->optiq_graph_init(self->topo_info);
}

void optiq_graph_construct(struct optiq_graph *self, optiq_graph_info *graph_info) 
{
    self->graph_impl->optiq_graph_contruct(self->topo_info, graph_info);
}

void optiq_graph_coarsen(struct optiq_graph *self, optiq_graph_info *graph_info)
{
    self->graph_impl->optiq_graph_coarsen(graph_info);
}

void optiq_graph_uncoarsen(struct optiq_graph *sefl, optiq_graph_info *graph_info)
{
    self->graph_impl->optiq_graph_uncoarsen(graph_info);
}

