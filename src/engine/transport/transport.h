#ifndef OPTIQ_TRANSPORT_H
#define OPTIQ_TRANSPORT_H

#include "transport_interface.h"

#include "pami/transport_pami.h"
#include "ugni/transport_ugni.h"
#include "nonblk_mpi/transport_nonblk_mpi.h"

struct transport {
    struct transport_interface *transport_impl;
};

void optiq_transport_init();

void optiq_transport_assign_service_level_to_message(struct optiq_message  message, int service_level);

void optiq_transport_add_message_to_hi_queue(struct optiq_message message, int weight);

void optiq_transport_add_message_to_low_queue(struct optiq_message message, int weight);

#endif
