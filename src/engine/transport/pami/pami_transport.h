#ifndef OPTIQ_PAMI_TRANSPORT_H
#define OPTIQ_PAMI_TRANSPORT_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#include <pami.h>
#endif

#include "../transport_interface.h"

extern struct optiq_transport_interface optiq_pami_transport_implementation;

struct optiq_pami_transport {
#ifdef __bgq__
    pami_client_t client;
    pami_context_t context;
#endif
    size_t num_contexts;
};

void optiq_pami_transport_init(struct optiq_transport *self);

void optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message &message);

#endif
