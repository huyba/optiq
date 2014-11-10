#include "nonblk_mpi_transport.h"

struct optiq_transport_interface optiq_nonblk_mpi_transport_implementation = {
    /*.init = */optiq_nonblk_mpi_transport_init,
    /*.send = */optiq_nonblk_mpi_transport_send
};

void optiq_nonblk_mpi_transport_init(struct optiq_transport *self)
{

}

int optiq_nonblk_mpi_transport_send(struct optiq_transport *self, struct optiq_message &message)
{

}
