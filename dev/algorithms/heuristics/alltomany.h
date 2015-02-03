#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

#include "path.h"
#include "heap_path.h"

struct multibfs {
    int num_dims;
    int num_nodes;
    int size[5];
    struct heap_path *heap;
    std::vector<int> *neighbors;
    struct path *paths;
    std::vector<struct path *> *edge_path;
};

void optiq_alg_heuristic_search_alltomany(std::vector<struct path *> &complete_paths, int num_dests, int *dests, struct multibfs *bfs);

#endif
