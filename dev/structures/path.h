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
    int hpos;

    std::vector<struct arc> arcs;

    int job_id;
    int path_id;
    int nd_id;
    int flow;
    int source_rank;
    int dest_rank;
};

extern int max_path_length;

int optiq_path_compare(struct path *p1, struct path *p2);

int optiq_path_compare_by_max_load(struct path *p1, struct path *p2);

void optiq_path_print_path(struct path *p);

void optiq_path_print_paths(std::vector<struct path*> &paths);

void optiq_path_print_stat(std::vector<struct path*> &paths, int num_nodes);

void optiq_path_read_from_file(char *filePath, std::vector<struct path *> &complete_paths);

void optiq_path_assign_ids(std::vector<struct path *> &complete_paths);

void optiq_path_reverse_paths (std::vector<struct path *> &complete_paths);

#endif
