#include "optiq.h"
#include <mpi.h>

void optiq_init(int argc, char **argv)
{
    optiq_topology_init();
    optiq_pami_transport_init();
    optiq_schedule_init();
    optiq_algorithm_init();

    topo->num_ranks_per_node = pami_transport->size/topo->num_nodes;

    MPI_Init(&argc, &argv);

    optiq_print_basic();
    optiq_opi_clear();
}

void optiq_print_basic ()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct topology *topo = optiq_topology_get();

    if (pami_transport->rank == 0) {
        optiq_topology_print_basic (topo);
    }
}

void optiq_finalize()
{
    optiq_topology_finalize();
    optiq_pami_transport_finalize();
    optiq_algorithm_finalize();
    optiq_schedule_finalize();

    MPI_Finalize();
}

void optiq_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    optiq_schedule_build (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    //optiq_pami_transport_execute (pami_transport);

    optiq_schedule_destroy ();
}

void optiq_mton_from_file(char *mtonfile)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    void *sendbuf = NULL, *recvbuf = NULL;
    int *sendcounts, *recvcounts, *sdispls, *rdispls;

    optiq_patterns_alltoallv_from_file(mtonfile, sched->jobs, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, rank, size);

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (sendbuf != NULL) {
	free(sendbuf);
    }
    if (recvbuf != NULL) {
	free(recvbuf);
    }

    free(sendcounts);
    free(recvcounts);
    free(sdispls);
    free(rdispls);
}

void optiq_mton_from_file_and_buffers (void *sendbuf, int *sdispls, void *recvbuf, int *rdispls, char *mtonfile)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct schedule *sched = optiq_schedule_get();

    std::vector<struct job> &jobs = sched->jobs;

    optiq_patterns_read_requests_from_file (mtonfile, requests);

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int *sendcounts = (int *) calloc (1, sizeof(int) * size);
    int *recvcounts = (int *) calloc (1, sizeof(int) * size);

    optiq_patterns_convert_requests_to_sendrecvcounts (jobs, sendcounts, recvcounts, rank);

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    free (sendcounts);
    free (recvcounts);
}
