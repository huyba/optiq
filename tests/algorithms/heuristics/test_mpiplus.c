#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "util.h"
#include "topology.h"
#include "path.h"
#include "mpiplus.h"
#include "patterns.h"

void optiq_test_alg_heuristic_mpiplus (std::vector<std::pair<int, std::vector<int> > > source_dests)
{
    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    optiq_alg_heuristic_search_mpiplus (complete_paths, source_dests);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    if (complete_paths.size() > source_dests.size()) 
    {
	printf("optiq %d paths more than mpi %d paths\n", complete_paths.size(), source_dests.size());
	//optiq_path_print_paths_coords (complete_paths, topo->all_coords);
        optiq_path_print_stat(complete_paths, bfs->num_nodes, topo->num_edges);
    }
    else
    {
	printf("Just like mpi path\n\n");
    }
}

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
	int n = bfs->num_nodes;
	std::vector<std::pair<int, std::vector<int> > > source_dests;

	printf("Dijoint testings\n");
	for (int i = 2; i <= 8; i *= 2)
	{
	    for (int j = 1; j < i; j++) 
	    {
		for (int r = 1; r <= 4; r *= 2)
		{
		    optiq_pattern_m_to_n_to_vectors (n/i, 0, n/i/r, j*n/i, source_dests);
		    optiq_test_alg_heuristic_mpiplus (source_dests);
		}
	    }
	}

	printf("Overlap testings\n");
	for(int i = 2; i <= 8; i *= 2)
        {
            for (int j = 0; j < i - 1; j++)
            {
		for (int k = 2; k <= 4; k *= 2)
		{
		    for (int r = 1; r <= 4; r *= 2)
		    {
			optiq_pattern_m_to_n_to_vectors (n/i, j*n/i, n/i/r, j*n/i + n/i - n/i/r/k, source_dests);
			optiq_test_alg_heuristic_mpiplus (source_dests);
		    }
		}
            }
        }

	printf("Subset testings\n");
	for(int i = 2; i <= 8; i *= 2)
        {
            for (int j = 0; j < i; j++)
            {
                for (int k = 2; k <= 4; k *= 2)
                {
                    for (int r = 2; r <= 4; r *= 2)
                    {
                        optiq_pattern_m_to_n_to_vectors (n/i, j*n/i, n/i/r, j*n/i + n/i/r/k, source_dests);
                        optiq_test_alg_heuristic_mpiplus (source_dests);
                    }
                }
            }
        }
    }

    return 0;
}

