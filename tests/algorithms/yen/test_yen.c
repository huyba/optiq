#include "yen.h"
#include "multibfs.h"
#include "multipaths.h"
#include <mpi.h>
#include <vector>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    optiq_multibfs_init ();
    struct multibfs *bfs = optiq_multibfs_get();

    std::vector<struct path *> complete_paths;
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int num_paths = 3;
    char *graphFilePath = "graph";

    for (int i = 0; i < size/2; i++) 
    {
	int dest_id = (i+size/2)/8;
	std::vector<int> d;
	d.push_back(dest_id);
	std::pair<int, std::vector<int> > p = make_pair(i/8, d);
	source_dests.push_back(p);
    }
    
    if (rank == 0) {
	optiq_graph_print_graph(bfs, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    get_yen_k_shortest_paths (complete_paths, source_dests, num_paths, graphFilePath);

    if (rank == 0) {
	optiq_path_print_paths(complete_paths);
    }
}
