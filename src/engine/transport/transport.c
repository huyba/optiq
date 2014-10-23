#include <stdlib.h>

#include <transport.h>

void optiq_transport_init(struct transport *self, enum transport_type type)
{
    if(type == PAMI) {
	self->transport_impl = &transport_pami;
    } else if (type == NONBLK_MPI) {
	self->transport_impl = &transport_nonblk_mpi;
    } else if (type == UGNI) {
	self->transport_impl = &transport_ugni;
    } else {
	/*self->transport_impl = &transport_tcp_ip;*/
    }
    self->transport_impl->optiq_transport_init();
}

void optiq_transport_assign_service_level_to_message(struct optiq_message  message, int service_level)
{
    message.service_level = service_level;
}

void optiq_transport_add_message_to_hi_queue(struct optiq_message message, int weight)
{

}

void optiq_transport_add_message_to_low_queue(struct optiq_message message, int weight)
{

}
