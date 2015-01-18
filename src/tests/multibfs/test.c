#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "topology.h"
#include "path.h"
#include "multibfs.h"

int main(int argc, char **argv)
{
    struct multibfs bfs;

    int num_dims = 5;
    int size[5] = {2, 4, 4, 4, 2};
    int num_nodes = 256;

    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

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

    build_paths(complete_paths, num_dests, dests, &bfs);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Build done in %ld microseconds\n", diff);

    //optiq_path_print_paths(complete_paths);
    //optiq_path_print_stat(complete_paths, num_nodes);

    return 0;
}

