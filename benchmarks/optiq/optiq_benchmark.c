#include <limits.h>

#include "optiq.h"
#include "opi.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

#include <mpi.h>

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths)
{
    int world_size, rank;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
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

    opi.numpaths.total = mpi_paths.size();
    opi.numpaths.avg = 1;   
    opi.numpaths.max = 1;
    opi.numpaths.min = 1;
    opi.numpaths.med = 1;

    if (odp.print_mpi_paths)
    {
	if (rank == 0)
	{
	    optiq_path_print_paths(mpi_paths);
	}

	MPI_Barrier(MPI_COMM_WORLD);
    }
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

    if (rank == 0) 
    {
	std::map<int, int>::iterator it;

        optiq_path_compute_stat (mpi_paths, size, topo->num_edges);

	for (int i = 0; i < mpi_paths.size(); i++)
	{
	    int hopbytes = mpi_paths[i]->arcs.size() * opi.message_size;
	    it = opi.path_hopbyte.find(hopbytes);

	    if (it == opi.path_hopbyte.end())
	    {
		opi.path_hopbyte.insert(std::pair<int, int>(hopbytes, 1));
	    }
	    else
	    {
		it->second++;
	    }
	}

	opi.hopbyte.max = 0;
	opi.hopbyte.min = INT_MAX;
	opi.hopbyte.avg = 0;
	opi.hopbyte.total = 0;
	opi.hopbyte.med = 0;
	int medindex = 0;

	for (it = opi.path_hopbyte.begin(); it != opi.path_hopbyte.end(); it++)
	{
	    opi.hopbyte.total += (double)it->first * (double)it->second;

	    if (opi.hopbyte.min > it->first && it->first > 0)
	    {
		opi.hopbyte.min = it->first;
	    }

	    if (opi.hopbyte.max < it->first)
	    {
		opi.hopbyte.max = it->first;
	    }

	    medindex += it->second;
	    if (medindex > mpi_paths.size()/2 && opi.hopbyte.med == 0)
	    {
		opi.hopbyte.med = it->first;
	    }
	}

	opi.hopbyte.avg = opi.hopbyte.total/mpi_paths.size();   
    }

    MPI_Barrier(MPI_COMM_WORLD);
}

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size)
{
    struct optiq_topology *topo = optiq_topology_get();
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
	optiq_path_compute_stat (opi.paths, size, topo->num_edges);
    }

    if (odp.print_timestamp) {
	optiq_opi_timestamp_print (rank);
    }

    optiq_opi_clear();
}

bool optiq_benchmark_jobs_from_file (char *jobfile, int datasize)
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
     
    /* When reading from file, always return jobs with path of node ids, not ranks */   
    optiq_jobs_read_from_file (jobs, path_ranks, jobfile);

    if (jobs.size() == 0 || path_ranks.size() == 0) 
    {
        if (rank == 0) {
            printf("No jobs or paths found from file %s\n", jobfile);
        }

        return false;
    }

    /* Convert to have paths with node ids and jobs with ranks */
    optiq_jobs_convert_ids_to_ranks (jobs, path_ids, topo->num_ranks_per_node);

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < jobs.size(); i++)
    {
        jobs[i].demand = datasize;
    }

    if (rank == 0 && odp.print_path_rank)
    {
	optiq_path_print_paths(schedule->paths);
    }

    if (rank == 0 && odp.print_job)
    {
	optiq_job_print(jobs, rank);
    }

    /* Check if enough paths to send, if not return. This should not happen, but still in current alg. */
    for (int i = 0; i < jobs.size(); i++) 
    {
	if (jobs[i].paths.size() == 0)
	{
	    for (int j = 0; j < path_ranks.size(); j++)
	    {
		free (path_ranks[i]);
	    }

	    if (rank == 0)
	    {
		printf("Not enought paths\n");
	    }

	    return false;
	}
    }

    if (rank == 0) 
    {
	printf("%s\n", jobs[0].name);
    }

    int *sendcounts, *sdispls, *recvcounts, *rdispls;
    char *sendbuf = NULL, *recvbuf = NULL;

    optiq_input_convert_jobs_to_alltoallv (jobs, &sendbuf, &sendcounts, &sdispls, &recvbuf, &recvcounts, &rdispls, size, rank);

    if (rank == 0) 
    {
	sprintf(opi.prefix, "%s", "M");
	opi.test_id = sched->test_id;
	sprintf(opi.name, "%s", "MPI_Alltoallv");
	opi.message_size = datasize;
	opi.chunk_size = schedule->chunk_size;
    }
    
    optiq_benchmark_mpi_perf(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (rank == 0) {
	optiq_path_compute_link_load (opi.load_stat, datasize);
	optiq_opi_print();
    }

    if (odp.print_mem_avail)
    {
	optiq_util_print_mem_info(rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_scheduler_build_schedule (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, jobs, path_ranks);

    optiq_opi_jobs_stat(jobs);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (odp.print_done_status) {
	printf("Rank %d Schedule done.\n", rank);
    }

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    if (odp.print_done_status) {
        printf("Rank %d Memory exchange done.\n", rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    if (odp.print_done_status) {
	printf("Rank %d done transport\n", rank);
    }

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

    return true;
}


bool optiq_benchmark_jobs (std::vector<struct job> &jobs)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;
    int size = pami_transport->size;

    if (odp.print_mem_avail) 
    {
	optiq_util_print_mem_info(rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0 && odp.print_path_rank)
    {
	optiq_path_print_paths(schedule->paths);
    }

    if (rank == 0 && odp.print_job)
    {
	optiq_job_print(jobs, rank);
    }

    if (rank == 0) 
    {
	printf("%s\n", jobs[0].name);
    }

    int *sendcounts, *sdispls, *recvcounts, *rdispls;
    char *sendbuf = NULL, *recvbuf = NULL;

    optiq_input_convert_jobs_to_alltoallv (jobs, &sendbuf, &sendcounts, &sdispls, &recvbuf, &recvcounts, &rdispls, size, rank);

    if (rank == 0) 
    {
	sprintf(opi.prefix, "%s", "M");
	opi.test_id = schedule->test_id;
	sprintf(opi.name, "%s", "MPI_Alltoallv");
	//opi.message_size = datasize;
	opi.chunk_size = schedule->chunk_size;
    }
    
    optiq_benchmark_mpi_perf(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (rank == 0) {
	//optiq_path_compute_link_load (opi.load_stat, datasize);
	optiq_opi_print();
    }

    if (odp.print_mem_avail)
    {
	optiq_util_print_mem_info(rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_scheduler_build_schedule (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, jobs, schedule->paths);

    optiq_opi_jobs_stat(jobs);

    MPI_Barrier(MPI_COMM_WORLD);
    
    if (odp.print_done_status) {
	printf("Rank %d Schedule done.\n", rank);
    }

    optiq_pami_transport_exchange_memregions ();

    MPI_Barrier(MPI_COMM_WORLD);

    if (odp.print_done_status) {
        printf("Rank %d Memory exchange done.\n", rank);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_pami_transport_execute_new ();

    if (odp.print_done_status) {
	printf("Rank %d done transport\n", rank);
    }

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

    return true;
}
