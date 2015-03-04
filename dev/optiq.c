#include "optiq.h"
#include <mpi.h>

void optiq_init(int argc, char **argv)
{
    optiq_topology_init();
    optiq_pami_transport_init();
    optiq_multibfs_init();
    optiq_schedule_init();

    MPI_Init(&argc, &argv);

    if (pami_transport->rank == 0) {
        optiq_topology_print_basic (topo);
    }
}

void optiq_finalize()
{
    optiq_topology_finalize();
    optiq_pami_transport_finalize();
    optiq_multibfs_finalize();
    optiq_schedule_finalize();

    MPI_Finalize();
}

void optiq_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    optiq_schedule_build (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute (pami_transport);

    optiq_schedule_destroy ();
}
