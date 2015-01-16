#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "path.h"
#include "multibfs.h"

int main(int argc, char **argv)
{
    struct multibfs bfs;

    int size[5] = {2, 4, 4, 4, 2};
    int num_nodes = 256;

    bfs.num_dims = 5;
    for (int i = 0; i < 5; i++) {
        bfs.size[i] = size[i];
    }

    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    /*Create a heap of paths*/
    bfs.heap = (struct heap_path *) malloc (sizeof (struct heap_path));
    hp_create(bfs.heap, num_nodes * num_dests);

    bfs.edge_path = (std::vector<struct path *> **) malloc (sizeof(std::vector<struct path *> *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
	bfs.edge_path[i] = (std::vector<struct path *> *) calloc (1, sizeof(std::vector<struct path *>) * num_nodes);
    }

    multibfs_init(&bfs);

    printf("Init done\n");

    std::vector<struct path *> complete_paths;

    struct timeval t1, t2;

    complete_paths.clear();

    gettimeofday(&t1, NULL);
    build_paths(complete_paths, num_dests, dests, &bfs);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Build done in %ld microseconds\n", diff);

    optiq_path_print_paths(complete_paths);
    optiq_path_print_stat(complete_paths, num_nodes);

    return 0;
}

