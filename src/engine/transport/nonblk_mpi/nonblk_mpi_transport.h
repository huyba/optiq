#ifndef OPTIQ_NONBLK_MPI_TRANSPORT_H
#define OPTIQ_NONBLK_MPI_TRANSPORT_H

#include "../transport_interface.h"

extern struct optiq_transport_interface optiq_nonblk_mpi_transport_implementation;

struct optiq_nonblk_mpi_transport {
    size_t num_contexts;
};

void optiq_nonblk_mpi_transport_init(struct optiq_transport *self);

void optiq_nonblk_mpi_transport_send(struct optiq_transport *self, struct optiq_message &message);

#endif
