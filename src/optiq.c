#include "optiq.h"

struct optiq *opq;

void optiq_init()
{
    opq = (struct optiq *)core_memory_alloc(sizeof(struct optiq), "optiq", "optiq_init");

    optiq_transport_init(&opq->transport, PAMI);
    opq->transport.virtual_lanes = &opq->virtual_lanes;
    opq->transport.jobs = &opq->jobs;
}

void optiq_create_communication_graph(int source, int dest, int nbyte, void *buffer)
{

}

void optiq_execute()
{

}
