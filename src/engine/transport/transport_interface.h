#ifndef OPTIQ_TRANSPORT_INTERFACE_H
#define OPTIQ_TRANSPORT_INTERFACE_H

#include "../virtuallane.h"

struct optiq_message;
struct optiq_transport;

struct optiq_transport_interface {
    void (*init)(struct optiq_transport *self);
    void (*send)(struct optiq_transport *self, struct optiq_message &message);
};

#endif
