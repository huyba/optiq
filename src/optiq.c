#include <stdio.h>
#include <stdlib.h>

#include "optiq.h"

struct optiq *opq;

void optiq_init()
{
    machine_type machine = XC30;
    opq = (struct optiq *)malloc(sizeof(struct optiq));
    opq->topo = (struct optiq_topology *)malloc(sizeof(struct optiq_topology));
    optiq_topology_init(opq->topo, machine);

    opq->graph = (struct optiq_graph *)malloc(sizeof(struct optiq_graph));
    optiq_graph_init(opq->graph, machine);
}

void optiq_get_topology_from_file(char *filePath)
{
    optiq_topology_get_topology_from_file(opq->topo, filePath);
}

void optiq_generate_model_data(char *filePath)
{
    optiq_graph_construct(opq->graph, opq->graph->graph);
}

