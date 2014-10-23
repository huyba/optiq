#ifndef OPTIQ_H
#define OPTIQ_H

#include <core/system/topology.h>
#include <core/structures/graph.h>
#include <model/request.h>
#include <engine/transport/transport.h>

struct optiq {
    machine_type machine;
    optiq_topology *topo;
};

extern struct optiq *opq;

void optiq_init();
void optiq_get_topology_from_file(char *filePath);

#endif
