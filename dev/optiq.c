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

    optiq_pami_transport_execute (pami_transport);

    optiq_schedule_destroy ();
}

void optiq_mton_from_file(char *mtonfile)
{
    std::vector<std::pair<std::pair<int, int>, int > > requests;

    optiq_patterns_read_from_file (mtonfile, requests);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int *sendcounts = (int *) calloc (1, sizeof(int) * size);
    int *recvcounts = (int *) calloc (1, sizeof(int) * size);

    optiq_patterns_convert_from_ssd_to_mpialltoallv(requests, sendcounts, recvcounts, rank);

    int *sdispls = (int *) calloc (1, sizeof(int) * size);
    int *rdispls = (int *) calloc (1, sizeof(int) * size);

    int sbytes = 0;
    for (int i = 0; i < size; i++)
    {
	if (sendcounts[i] != 0) 
	{
	    sdispls[i] = sbytes;
	    sbytes += sendcounts[i];
	    printf("Rank %d, send sbytes = %d, dest = %d\n", rank, sendcounts[i], i);
	}
    }

    void *sendbuf = NULL;

    if (sbytes > 0) {
	sendbuf = malloc (sbytes);
    }

    int rbytes = 0;
    for (int i = 0; i < size; i++)
    {
	if (recvcounts[i] != 0)
	{
	    rdispls[i] = rbytes;
	    rbytes += recvcounts[i];
	    printf("Rank %d, recv rbytes = %d, source = %d\n", rank, recvcounts[i], i);
	}
    }

    void *recvbuf = NULL;

    if (rbytes > 0) {
	recvbuf = malloc (rbytes);
    }

    printf("Rank %d, sbytes = %d, rbytes = %d\n", rank, sbytes, rbytes);

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);
}

void optiq_mton_from_file_and_buffers (void *sendbuf, int *sdispls, void *recvbuf, int *rdispls, char *mtonfile)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    std::vector<std::pair<std::pair<int, int>, int > > requests;

    optiq_patterns_read_from_file (mtonfile, requests);

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int *sendcounts = (int *) calloc (1, sizeof(int) * size);
    int *recvcounts = (int *) calloc (1, sizeof(int) * size);

    optiq_patterns_convert_from_ssd_to_mpialltoallv(requests, sendcounts, recvcounts, rank);

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    free (sendcounts);
    free (recvcounts);
}
