#include <stdlib.h>

#include "transport.h"

void optiq_transport_init(struct optiq_transport *self, enum transport_type type)
{
    if(type == PAMI) {
	self->transport_implementation = &optiq_pami_transport_implementation;
    } else if (type == NONBLK_MPI) {
	self->transport_implementation = &optiq_nonblk_mpi_transport_implementation;
    } else if (type == GNI) {
	self->transport_implementation = &optiq_gni_transport_implementation;
    } else {
	/*self->transport_implementation = &optiq_tcp_ip_transport_implementation;*/
    }
    self->transport_implemetation->init();
}

void optiq_transport_send(struct optiq_transport *self, struct optiq_message &message)
{
    self->transport_implemetation->send(self, message);
}
