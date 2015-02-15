#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

#include "path.h"
#include "heap_path.h"

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

void optiq_multibfs_init(struct multibfs &bfs);

void add_load_on_path(struct path *np, int *load, int adding_load, int num_nodes);

void update_max_load(struct path *np, int *load, struct multibfs *bfs);

void add_edge_path(std::vector<struct path*> *edge_path, struct path *p, int num_nodes);

#endif
