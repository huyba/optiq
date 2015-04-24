#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "topology.h"
#include "path.h"
#include "manytomany.h"
#include "cesm.h"

void gen_source_dests_test(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs)
{
    source_dests.clear();

    int num_dests = bfs.num_nodes/64;
    std::vector<int> dests;
    dests.clear();

    for (int i = 0; i < num_dests; i++) {
        dests.push_back(i * 64 + 32);
    }

    for (int i = 0; i < bfs.num_nodes; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, dests);
        source_dests.push_back(p);
    }
}

void search_paths(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs, std::vector<struct path *> &complete_paths)
{
    complete_paths.clear();

    max_path_length = bfs.diameter/2;

    printf("Done init\n");

    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    optiq_alg_heuristic_search_manytomany_late_adding_load (complete_paths, source_dests, &bfs);

    gettimeofday(&t2, NULL);

    long int diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Build done in %ld microseconds\n", diff);

    //optiq_path_print_paths(complete_paths);
    optiq_path_print_stat(complete_paths, bfs.num_nodes, 0);
}

int main(int argc, char **argv)
{
    struct multibfs bfs;

    int size[5] = {4, 4, 4, 4, 2};
    if (argc > 5)
    {
        size[0] = atoi(argv[1]);
        size[1] = atoi(argv[2]);
        size[2] = atoi(argv[3]);
        size[3] = atoi(argv[4]);
        size[4] = atoi(argv[5]);
    }

    bfs.num_dims = 5;
    bfs.num_nodes = 1;
    bfs.diameter = 0;

    for (int i = 0; i < bfs.num_dims; i++) {
        bfs.size[i] = size[i];
        bfs.num_nodes *= size[i];
        bfs.diameter += size[i];
    }

    bfs.neighbors = optiq_topology_get_all_nodes_neighbors(bfs.num_dims, size);

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    source_dests.clear();

    //gen_source_dests_test(source_dests, bfs);
    optiq_cesm_gen_ice_cpl(source_dests, bfs.num_nodes);

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    search_paths(source_dests, bfs, complete_paths);

    optiq_path_print_paths(complete_paths);   

    return 0;
}

