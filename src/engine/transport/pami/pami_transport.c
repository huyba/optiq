#include "transport_pami.h"

struct optiq_transport_interface optiq_pami_transport_implementation = {
    .init = optiq_pami_transport_init,
    .send = optiq_pami_transport_send       
};

void optiq_pami_transport_init(struct optiq_transport *self, enum transport_type type)
{

}

void optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message &message)
{

}

