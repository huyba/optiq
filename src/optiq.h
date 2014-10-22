#ifndef OPTIQ_H
#define OPTIQ_H

#include <core/system/topology.h>
#include <core/structures/graph.h>
#include <model/request.h>
#include <engine/transport.h>

struct optiq {
    machine_type machine;
};

void optiq_init();

#endif
