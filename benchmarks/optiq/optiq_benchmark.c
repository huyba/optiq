#include "optiq.h"
#include "opi.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

#include <mpi.h>

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths)
{
    int world_size;

    MPI_Comm_size (MPI_COMM_WORLD, &world_size);

    std::vector<struct job> jobs;
    optiq_input_convert_sendcounts_to_jobs (sendcounts, &jobs, world_size, topo->num_ranks_per_node);

    mpi_paths.clear();

    std::vector<std::pair<int, int> > source_dests;
    source_dests.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
	std::pair<int, int> p = std::make_pair (jobs[i].source_id, jobs[i].dest_id);
	source_dests.push_back(p);
    }

    optiq_topology_path_reconstruct_new (source_dests, mpi_paths);
}

void optiq_benchmark_mpi_perf(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    MPI_Barrier (MPI_COMM_WORLD);

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    std::vector<struct path *> mpi_paths;
    mpi_paths.clear();
    optiq_benchmark_reconstruct_mpi_paths(sendcounts, mpi_paths);

    int rank, size;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    if (rank == 0) {
        optiq_path_print_stat (mpi_paths, size, topo->num_edges);
    }

    MPI_Barrier(MPI_COMM_WORLD);
}

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size)
{
    struct topology *topo = optiq_topology_get();
    struct optiq_schedule *sched = optiq_schedule_get();

    void *sendbuf = NULL, *recvbuf = NULL;
    int *sendcounts, *sdispls, *recvcounts, *rdispls;

    optiq_patterns_alltoallv_from_file (filepath, sched->jobs, sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, rank, size);

    if (rank == 0) {
        printf("\nTest A - MPI\n\n");
    }

    optiq_benchmark_mpi_perf(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (rank == 0) {
        printf("Test B - OPTIQ\n\n");
    }

    optiq_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect ();

    if (rank == 0) 
    {
	if (mpi_time > max_opi.transfer_time) 
	{
	    printf("Bingo mpi_time = %8.0fd optiq time = %8.0f\n", mpi_time, max_opi.transfer_time);
	}
	optiq_opi_print();
    }

    if (rank == 0) {
	optiq_path_print_stat (opi.paths, size, topo->num_edges);
	/*optiq_path_print_paths_coords (opi.paths, topo->all_coords);*/
    }

    if (odp.print_timestamp) {
	optiq_opi_timestamp_print (rank);
    }

    optiq_opi_clear();
}

void optiq_benchmark_jobs_from_file (char *jobfile, int datasize)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    if (odp.print_mem_avail) 
    {
	optiq_util_print_mem_info(rank);
    }

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

    if (rank == 0) 
    {
	printf("%s\n", jobs[0].name);
    }

    int *sendcounts, *sdispls, *recvcounts, *rdispls;
    char *sendbuf = NULL, *recvbuf = NULL;

    optiq_input_convert_jobs_to_alltoallv (jobs, &sendbuf, &sendcounts, &sdispls, &recvbuf, &recvcounts, &rdispls, size, rank);
    
    optiq_benchmark_mpi_perf(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (odp.print_mem_avail)
    {
	optiq_util_print_mem_info(rank);
    }

    optiq_scheduler_build_schedule (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, jobs, path_ranks);

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

    optiq_pami_transport_clear();

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

	free (testbuf);
	free (recvbuf);
    }
    
    free (sendcounts);
    free (recvcounts);
    free (rdispls);
    free (sdispls);

    if (schedule->send_len > 0) {
	free (sendbuf);
    }

    if (odp.print_mem_avail)
    {
	optiq_util_print_mem_info(rank);
    }
}
