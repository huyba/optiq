#include "optiq.h"
#include <mpi.h>
#include "optiq_benchmark.h"

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

    optiq_schedule_clear ();
}

void optiq_mton_from_file(char *mtonfile)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *sched = optiq_schedule_get();

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
    struct optiq_schedule *sched = optiq_schedule_get();

    std::vector<struct job> &jobs = sched->jobs;

    optiq_patterns_read_requests_from_file (mtonfile, jobs);

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int *sendcounts = (int *) calloc (1, sizeof(int) * size);
    int *recvcounts = (int *) calloc (1, sizeof(int) * size);

    optiq_patterns_convert_requests_to_sendrecvcounts (jobs, sendcounts, recvcounts, rank);

    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    free (sendcounts);
    free (recvcounts);
}

void optiq_execute_jobs_from_file (char *jobfile, int datasize)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    struct optiq_schedule *sched = optiq_schedule_get();

    std::vector<struct job> &jobs = sched->jobs;
    jobs.clear();
    std::vector<struct path *> &paths = sched->paths;
    paths.clear();

    optiq_jobs_read_from_file (jobs, paths, jobfile);

    if (rank == 0) {
	printf("%s\n", jobs[0].name);
    }

    int *sendcounts = (int *) calloc (1, sizeof(int) * size);
    int *recvcounts = (int *) calloc (1, sizeof(int) * size);
    int *sdispls = (int *) calloc (1, sizeof(int) * size);
    int *rdispls = (int *) calloc (1, sizeof(int) * size);

    char *sendbuf, *recvbuf, *testbuf;

    int sendbytes = 0, recvbytes = 0;

    for (int i = 0; i < jobs.size(); i++) 
    {
	if (jobs[i].source_rank == rank)
	{
	    sendcounts[jobs[i].dest_rank] = datasize;
	    sdispls[jobs[i].dest_rank] = sendbytes;
	    sendbytes += datasize;
	}

	if (jobs[i].dest_rank == rank)
	{
	    recvcounts[jobs[i].source_rank] = datasize;
	    rdispls[jobs[i].source_rank] = recvbytes;
	    recvbytes += datasize;
	}
    }

    if (sendbytes > 0)
    {
	sendbuf = (char *) malloc (sendbytes);
    
	for (int i = 0; i < sendbytes; i++) {
	    sendbuf[i] = i%128;
	}
    }

    if (recvbytes > 0) 
    {
	recvbuf = (char *) malloc (recvbytes);

	testbuf = (char *) malloc (recvbytes);
	for (int i = 0; i < recvbytes; i++) {
	    testbuf[i] = i % 128;
	}
    }

    optiq_benchmark_mpi_perf(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    optiq_schedule_build_new (jobs, paths, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
	printf("Schedule done.\n");
    }

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Memory exchange done.\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    optiq_schedule_clear ();

    if (recvbytes > 0) {
	if (memcmp (recvbuf, testbuf, recvbytes) != 0) {
	    printf("Rank %d encounter data corrupted\n", rank);
	}
    }
    
    free (sendcounts);
    free (recvcounts);    
}
