#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

#include "path.h"
#include "heap_path.h"

struct multibfs {
    int num_dims;
    int size[5];
    int **all_coords;
    int **graph;
    int **load;
    bool ** visited;
    struct heap_path *heap;
    std::vector<struct path *> **edge_path;
    std::vector<int> *neighbors;
    struct path *paths;
    int max_avail_path_id;
};

void multibfs_init(struct multibfs *bfs);

void build_paths(std::vector<struct path *> &complete_paths, int num_dests, int *dests, struct multibfs *bfs);

#endif
