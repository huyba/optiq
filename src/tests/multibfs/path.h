#ifndef OPTIQ_PATH
#define OPTIQ_PATH

#include <vector>

struct arc {
    int u;
    int v;
};

struct path {
    int max_load;
    int root_id;
    std::vector<struct arc> arcs;
    int hpos;
};

int optiq_path_compare(struct path *p1, struct path *p2);

void optiq_path_print_path(struct path *p);

void optiq_path_print_paths(std::vector<struct path*> &paths);

void optiq_path_print_stat(std::vector<struct path*> &paths, int num_nodes);

#endif
