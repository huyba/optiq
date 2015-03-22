#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "topology.h"
#include "path.h"
#include "mpiplus.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    optiq_topology_init();
    optiq_multibfs_init();

    struct topology *topo = optiq_topology_get();
    struct multibfs *bfs = optiq_multibfs_get();

    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);  

    if (rank == 0)
    {
	std::vector<std::pair<int, std::vector<int> > > source_dests;
	source_dests.clear();

	int num_nodes = bfs->num_nodes;

	for (int i = 0; i < 64; i++) 
	{
	    std::vector<int> dests;
	    dests.clear();

	    dests.push_back(num_nodes - 64 + i);

	    std::pair<int, std::vector<int> > sd = make_pair(i, dests);

	    source_dests.push_back(sd);
	}

	std::vector<struct path *> complete_paths;
	complete_paths.clear();

	struct timeval t1, t2;

	gettimeofday(&t1, NULL);

	optiq_alg_heuristic_search_mpiplus (complete_paths, source_dests);

	gettimeofday(&t2, NULL);

	long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

	printf("Build done in %ld microseconds\n", diff);

	//optiq_path_print_paths(complete_paths);
	optiq_path_print_stat(complete_paths, num_nodes, topo->num_edges);
    }

    return 0;
}

