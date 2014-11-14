#ifndef OPTIQ_TRANSPORT_INTERFACE_H
#define OPTIQ_TRANSPORT_INTERFACE_H

#include "../virtual_lane.h"

struct optiq_message;
struct optiq_transport;

struct optiq_transport_interface {
    void (*init)(struct optiq_transport *self);
    int (*send)(struct optiq_transport *self, struct optiq_message *message);
    int (*recv)(struct optiq_transport *self, struct optiq_message *message);
    bool (*test)(struct optiq_transport *self, struct optiq_job *job);
    int (*destroy)(struct optiq_transport *self);
};

#endif
