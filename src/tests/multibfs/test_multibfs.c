#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include "multibfs.h"

int main(int argc, char **argv)
{
    struct multibfs bfs;

    int size[5] = {2, 4, 4, 4, 2};

    bfs.num_dims = 5;
    for (int i = 0; i < 5; i++) {
	bfs.size[i] = size[i];
    }

    multibfs_init(&bfs);

    printf("Init done\n");

    std::vector<struct path> complete_paths;
    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    build_paths(complete_paths, num_dests, dests, &bfs);

    printf("Build done\n");

    optiq_path_print_paths(complete_paths);

    return 0;
}
