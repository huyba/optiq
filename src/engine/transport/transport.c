#include <stdlib.h>

#include "transport.h"

void optiq_transport_init(struct optiq_transport *self, enum optiq_transport_type type)
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
    self->type = type;
    self->transport_implementation->init(self);
}

void optiq_transport_send(struct optiq_transport *self, struct optiq_message &message)
{
    self->transport_implementation->send(self, message);
}

void* optiq_transport_get_concrete_transport(struct optiq_transport *self)
{
    if (self->concrete_transport == NULL) {
	if (self->type == PAMI) {
	    self->concrete_transport = (struct optiq_pami_transport *)malloc(sizeof(struct optiq_pami_transport));
	} else if (self->type == GNI) {
	    self->concrete_transport = (struct optiq_gni_transport *)malloc(sizeof(struct optiq_gni_transport));
	} else if (self->type == NONBLK_MPI) {
	    self->concrete_transport = (struct optiq_nonblk_mpi_transport *)malloc(sizeof(struct optiq_nonblk_mpi_transport));
	} else {

	}
    }

    return self->concrete_transport;
}

