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

    bfs.num_dims = 5;
    for (int i = 0; i < 5; i++) {
        bfs.size[i] = size[i];
    }

    multibfs_init(&bfs);

    printf("Init done\n");

    std::vector<struct path> complete_paths;
    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    build_paths(complete_paths, num_dests, dests, &bfs);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Build done in %ld microseconds\n", diff);

    printf("Build done in %ld microseconds\n", diff);

    //optiq_path_print_paths(complete_paths);

    return 0;
}

