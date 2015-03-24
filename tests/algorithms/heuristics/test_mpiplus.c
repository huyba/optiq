#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "util.h"
#include "topology.h"
#include "path.h"
#include "mpiplus.h"

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

void optiq_pattern_m_to_n_to_vectors (int m, int startm, int n, int startn, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    printf("%d ranks from %d to %d talks to %d rank from %d to %d\n", m, startm, startm + m - 1, n, startn, startn + n - 1);

    if (m > n)
    {
        int r = m/n;
        int d = startn;

        for (int i = startm; i < m + startm; i += r)
        {
	    std::vector<int> dests;
	    dests.clear();

            for (int j = 0; j < r; j++)
            {
		dests.push_back (j);
            }

	    std::pair<int, std::vector<int> > sd = std::make_pair (i, dests);
	    source_dests.push_back (sd);

            d++;
        }
    }
    else
    {
        int r = n/m;
        int d = startn;

        for (int i = startm; i < m +  startm; i++)
        {
	    std::vector<int> dests;
	    dests.clear();

            for (int j = 0; j < r; j++)
            {
		 dests.push_back (d + j);
            }

	    std::pair<int, std::vector<int> > sd = std::make_pair (i, dests);
	    source_dests.push_back (sd);

            d += r;
        }
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

	for (int i = 2; i <= 8; i *= 2)
	{
	    for (int j = 1; j < i; j++) 
	    {
		optiq_pattern_m_to_n_to_vectors (n/i, 0, n/i, j*n/i, source_dests);
		optiq_test_alg_heuristic_mpiplus (source_dests);
	    }
	}
    }

    return 0;
}

