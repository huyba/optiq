#include <stdio.h>
#include <stdlib.h>

#include "optiq.h"

struct optiq *opq;

void optiq_init()
{
    machine_type machine = XC30;
    opq = (struct optiq *)malloc(sizeof(struct optiq));
    opq->topo = (optiq_topology *)malloc(sizeof(struct optiq_topology));
    optiq_topology_init(opq->topo, machine);
}

void optiq_get_topology_from_file(char *filePath)
{
    optiq_topology_get_topology_from_file(opq->topo, filePath);
}

