#include "optiq.h"
#include <mpi.h>

/*
 * Init the optiq, includes:
 * 1. topology
 * 2. transport
 * 3. schedule
 * 4. algorithm
 * 5. mpi
 * 6. print topo info
 * 7. clear the statistic info.
 * */
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


/*
 * Transfer data in the form of alltoallv format.
 * */
void optiq_alltoallv (void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int num_ranks = pami_transport->size;
    struct optiq_schedule *sched = optiq_schedule_get();

    std::vector<struct job> &jobs = sched->jobs;
    std::vector<struct path *> &path_ids = opi.paths, &path_ranks = sched->paths;
    jobs.clear();
    path_ids.clear();
    path_ranks.clear();

    optiq_input_convert_sendcounts_to_jobs(sendcounts, &jobs, num_ranks, topo->num_ranks_per_node);
    
    optiq_algorithm_search_path (path_ids, jobs, bfs, rank);

    optiq_schedule_assign_path_ids_to_jobs (path_ids, jobs, path_ranks, topo->num_ranks_per_node);

    if (rank == 0) 
    {
	char filepath[256];
	jobs[0].name = filepath;
	sprintf(filepath, "test%d_%d", jobs[0].source_id, jobs[0].dest_id);
	optiq_job_write_to_file (jobs, filepath);
    }

    optiq_scheduler_build_schedule (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, jobs, path_ranks);

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    optiq_schedule_clear ();
}

/* 
 * Transfer data with requests from file in the format of : source dest demand
 * */
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

/* 
 * Transfer data with requests from file in the format of : source dest demand with buffer provided.
 * */
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

/*
 * Transfer data with paths and jobs from file.
 * */
void optiq_execute_jobs_from_file (char *jobfile, int datasize)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    struct optiq_schedule *sched = optiq_schedule_get();

    std::vector<struct job> &jobs = sched->jobs;
    jobs.clear();
    std::vector<struct path *> &path_ids = opi.paths, &path_ranks = sched->paths;
    path_ranks.clear();

    optiq_jobs_read_from_file (jobs, path_ranks, jobfile);

    for (int i = 0; i < jobs.size(); i++) 
    {
	jobs[i].demand = datasize;
    }

    optiq_path_creat_path_ids_from_path_ranks(path_ids, path_ranks, topo->num_ranks_per_node);

    if (rank == 0) {
	printf("%s\n", jobs[0].name);
    }

    int *sendcounts, *sdispls, *recvcounts, *rdispls;
    char *sendbuf, *recvbuf;

    optiq_input_convert_jobs_to_alltoallv (jobs, &sendbuf, &sendcounts, &sdispls, &recvbuf, &recvcounts, &rdispls, size, rank);

    optiq_scheduler_build_schedule (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, jobs, path_ranks);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
	printf("Schedule done.\n");
    }

    optiq_pami_transport_info_status(pami_transport->transport_info, rank);

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Memory exchange done.\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    optiq_schedule_clear ();

    if (schedule->recv_len > 0) 
    {
	char *testbuf = (char *) malloc (schedule->recv_len);

	for (int i = 0; i < schedule->recv_len; i++) {
	    testbuf[i] = i % 128;
	}

	if (memcmp (recvbuf, testbuf, schedule->recv_len) != 0) {
	    printf("Rank %d encounter data corrupted\n", rank);
	}
    }
    
    free (sendcounts);
    free (recvcounts);
}
