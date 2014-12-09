#ifndef OPTIQ_H
#define OPTIQ_H

#include "core/structures/queue.h"
#include "core/structures/job.h"
#include "core/structures/flow.h"
#include "core/system/memory.h"
#include "engine/message.h"
#include "engine/virtual_lane.h"
#include "engine/transport/transport.h"

struct optiq {
    struct optiq_transport transport;
    vector<struct optiq_virtual_lane> virtual_lanes;
    vector<struct optiq_job> jobs;
};

extern struct optiq *opq;

void optiq_init();
void optiq_create_communication_graph(int source, int dest, int nbyte, void *buffer);
void optiq_schedule();
void optiq_execute();

#endif
