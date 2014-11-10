#ifndef OPTIQ_GNI_TRANSPORT_H
#define OPTIQ_GNI_TRANSPORT_H

#include "../transport_interface.h"

extern struct optiq_transport_interface optiq_gni_transport_implementation;

struct optiq_gni_transport {
};

void optiq_gni_transport_init(struct optiq_transport *self);

int optiq_gni_transport_send(struct optiq_transport *self, struct optiq_message &message);

#endif
