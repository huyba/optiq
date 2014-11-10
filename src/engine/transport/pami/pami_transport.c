#include "pami_transport.h"

struct optiq_transport_interface optiq_pami_transport_implementation = {
    .init = optiq_pami_transport_init,
    .send = optiq_pami_transport_send       
};

void optiq_pami_transport_init(struct optiq_transport *self)
{

}

void optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message &message)
{
    printf("Transport data of size %d to dest %d with flow_id = %d\n", message.length, message.dest, message.flow_id);
}

