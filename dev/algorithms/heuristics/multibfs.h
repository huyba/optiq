#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

struct multibfs {
    int num_dims;
    int num_nodes;
    int size[5];
    int diameter;
    struct heap_path *heap;
    std::vector<int> *neighbors;
    struct path *paths;
    std::vector<struct path *> *edge_path;
};

struct multibfs_perf {
    long int update_max_load_time;
    long int add_load_time;
    long int add_edge_path_time;
};

#endif
