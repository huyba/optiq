#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "topology.h"
#include "path.h"
#include "manytomany.h"

int main(int argc, char **argv)
{
    struct mtonbfs bfs;

    int num_dims = 5;
    int size[5] = {2, 4, 4, 8, 2};
    int num_nodes = 512;

    int num_dests = num_nodes/64;
    int *dest_ranks = (int *) malloc (sizeof(int) * num_dests);
    for (int i = 0; i < num_dests; i++) {
	dest_ranks[i] = i * 64 + 32;
    }

    int num_sources = num_nodes;
    int *source_ranks = (int *) malloc (sizeof(int) * num_sources);
    for (int i = 0; i < num_sources; i++) {
	source_ranks[i] = i;
    }

    std::vector<int> *neighbors = optiq_topology_get_all_nodes_neighbors(num_dims, size);

    bfs.num_dims = num_dims;
    bfs.num_nodes = num_nodes;
    for (int i = 0; i < 5; i++) {
        bfs.size[i] = size[i];
    }
    bfs.neighbors = neighbors;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    printf("Done init\n");

    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    optiq_path_search_manytomany(complete_paths, num_sources, source_ranks, num_dests, dest_ranks, &bfs);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Build done in %ld microseconds\n", diff);

    optiq_path_print_paths(complete_paths);
    optiq_path_print_stat(complete_paths, num_nodes);

    return 0;
}

