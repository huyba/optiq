#include "gni_transport.h"

struct optiq_transport_interface optiq_gni_transport_implementation = {
    /*.init = */optiq_gni_transport_init,
    /*.send = */optiq_gni_transport_send
};

void optiq_gni_transport_init(struct optiq_transport *self)
{

}

void optiq_gni_transport_send(struct optiq_transport *self, struct optiq_message &message)
{

}
