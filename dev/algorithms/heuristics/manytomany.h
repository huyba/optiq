#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

#include "path.h"
#include "heap_path.h"

struct mtonbfs {
    int num_dims;
    int num_nodes;
    int size[5];
    struct heap_path *heap;
    std::vector<int> *neighbors;
    struct path *paths;
    std::vector<struct path *> *edge_path;
};

void optiq_path_search_manytomany(std::vector<struct path *> &complete_paths, int num_sources, int *source_ranks, int num_dests, int *dest_ranks, struct mtonbfs *bfs);

#endif
