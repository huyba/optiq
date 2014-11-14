#ifndef OPTIQ_H
#define OPTIQ_H

#include "job.h"
#include "flow.h"
#include "transport.h"
#include "virtual_lane.h"

struct optiq {
    struct optiq_transport *transport;
    struct optiq_virtual_lane *virtual_lanes;
};

extern struct optiq *opq;

void optiq_init();
void optiq_schedule(int source, int dest, int nbyte, void *buffer);
void optiq_execute();

#endif
