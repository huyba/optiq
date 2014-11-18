#ifndef OPTIQ_H
#define OPTIQ_H

#include "job.h"
#include "flow.h"
#include "message.h"
#include "memory.h"
#include "transport.h"
#include "virtual_lane.h"

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
