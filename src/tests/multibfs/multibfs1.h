#ifndef OPTIQ_MULTIBFS_H
#define OPTIQ_MULTIBFS_H

#include <vector>

struct arc {
    int u;
    int v;
};

struct path {
    int max_load;
    int dest_id;
    std::vector<struct arc> arcs;
};

struct multibfs {
    int num_dims;
    int size[5];
    int **all_coords;
    int **graph;
    int **load;
    bool ** visited;
};

void optiq_path_print_paths(std::vector<struct path> &paths);

void optiq_path_print_stat(std::vector<struct path> &paths, int num_nodes);

void multibfs_init(struct multibfs *bfs);

void build_paths(std::vector<struct path> &complete_paths, int num_dests, int *dests, struct multibfs *bfs);

void naive_build_paths(std::vector<struct path> &complete_paths, int num_dests, int *dests, struct multibfs *bfs);

#endif
