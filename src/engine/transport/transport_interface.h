#ifndef OPTIQ_TRANSPORT_INTERFACE_H
#define OPTIQ_TRANSPORT_INTERFACE_H

#include "../virtuallane.h"

struct optiq_message;
struct optiq_transport;

enum optiq_transport_type {
    PAMI = 1,
    GNI = 2,
    NONBLK_MPI = 3
};

struct optiq_transport_interface {
    void (*init)(struct optiq_transport *self, enum optiq_transport_type type);
    void (*send)(struct optiq_transport *self, struct optiq_message &message);
};

#endif
