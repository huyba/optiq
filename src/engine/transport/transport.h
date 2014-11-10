#ifndef OPTIQ_TRANSPORT_H
#define OPTIQ_TRANSPORT_H

#include "transport_interface.h"

#include "pami/pami_transport.h"
#include "gni/gni_transport.h"
#include "nonblk_mpi/nonblk_mpi_transport.h"

enum optiq_transport_type {
    PAMI = 1,
    GNI = 2,
    NONBLK_MPI = 3
};

struct optiq_transport {
    struct optiq_transport_interface *transport_implementation;
    void *concrete_transport;
    optiq_transport_type type;
    int size;
    int rank;
};

void optiq_transport_init(struct optiq_transport *self, enum optiq_transport_type type);

void optiq_transport_send(struct optiq_transport *self, struct optiq_message &message);

void* optiq_transport_get_concrete_transport(struct optiq_transport *self);

#endif
