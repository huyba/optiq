#include "optiq.h"
#include "opi.h"
#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

#include <mpi.h>

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths)
{
    mpi_paths.clear();

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    source_dests.clear();

    optiq_schedule_get_pair (sendcounts, source_dests, NULL);

    /*if (pami_transport->rank == 0) {
	optiq_schedule_print_sourcedests(source_dests);
    }*/

    if (topo->num_ranks_per_node > 1) 
    {
	for (int i = 0; i < source_dests.size(); i++)
	{
	    source_dests[i].first = source_dests[i].first / topo->num_ranks_per_node;
	    for (int j = 0; j < source_dests[i].second.size(); j++)
	    {
		source_dests[i].second[j] = source_dests[i].second[j] / topo->num_ranks_per_node;
	    }
	}
    }

    optiq_topology_path_reconstruct_new (source_dests, mpi_paths);
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

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    std::vector<struct path *> mpi_paths;
    mpi_paths.clear();
    optiq_benchmark_reconstruct_mpi_paths(sendcounts, mpi_paths);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
	//optiq_path_print_paths_coords(mpi_paths, topo->all_coords);
	/*printf("#paths = %d, size = %d, #edges = %d\n", mpi_paths.size(), size, topo->num_edges);*/
        optiq_path_print_stat (mpi_paths, size, topo->num_edges);
    }

    if (rank == 0) {
        printf("Test B - OPTIQ\n\n");
    }

    optiq_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect ();

    if (rank == 0) {
	if (mpi_time > max_opi.transfer_time) {
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

