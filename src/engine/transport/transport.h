#ifndef OPTIQ_TRANSPORT_H
#define OPTIQ_TRANSPORT_H

#include "transport_interface.h"

#include "pami/pami_transport.h"
#include "gni/gni_transport.h"
#include "nonblk_mpi/nonblk_mpi_transport.h"

struct optiq_transport {
    struct optiq_transport_interface *transport_implementation;
    void *concret_transport;
};

void optiq_transport_init(struct transport *self, enum transport_type type);

void optiq_transport_send(struct transport *self, struct optiq_message &message);

#endif
