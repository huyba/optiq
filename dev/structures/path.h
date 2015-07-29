/*
 * Header for path, which is used to move data along from a source to a destination
 * */

#ifndef OPTIQ_PATH
#define OPTIQ_PATH

#include <vector>

/* arc data structure with 2 end verties */
struct arc {
    int u;
    int v;
};

/* path structure of a path from a source to a destination */
struct path {
    int max_load;
    int root_id;
    int hpos;

    std::vector<struct arc> arcs;

    int job_id;
    int path_id;
    int flow;
    int source_id;
    int dest_id;
    int source_rank;
    int dest_rank;
    int assigned_len;
    int nbytes;
    int copies;
};

extern int max_path_length;
extern int max_path_load;

/* Compare 2 paths that favors number of hops */
int optiq_path_compare_favor_hop (struct path *p1, struct path *p2);

/* Compare 2 paths with the comparison that favors load */
int optiq_path_compare_favor_load (struct path *p1, struct path *p2);

/* Default function to compare 2 paths*/
int optiq_path_compare(struct path *p1, struct path *p2);

/* Compare 2 paths by maxload */
int optiq_path_compare_by_max_load(struct path *p1, struct path *p2);

/* Write paths to file */
void optiq_path_write_paths(std::vector<struct path *> &paths, char *filepath);

/* Print a path */
void optiq_path_print_path(struct path *p);

/*Print path's load statistics */
void optiq_path_print_load_stat(int *load_stat);

/* Compute link load of paths's loadstat */
void optiq_path_compute_link_load (int *load_stat, int datasize);

/* Print a path with coordinates of nodes */
void optiq_path_print_path_coords(struct path *p, int** coords);

/* Print paths information */
void optiq_path_print_paths(std::vector<struct path*> &paths);

/* Print paths with coordinates of all nodes */
void optiq_path_print_paths_coords(std::vector<struct path *> &paths, int** coords);

/* Compute statistics for paths */
void optiq_path_compute_stat(std::vector<struct path*> &paths, int num_nodes, int num_edges);

/* Read flow values of paths */
bool optiq_path_read_flow_value_from_file (char *filePath, std::vector<struct job> &jobs);

/* Read only paths from a file */
void optiq_path_read_from_file(char *filePath, std::vector<struct path *> &complete_paths);

/* Assign unique ids for paths */
void optiq_path_assign_ids(std::vector<struct path *> &complete_paths);

/* Reverse a path, source <--> destination */
void optiq_path_reverse_paths (std::vector<struct path *> &complete_paths);

/* create the source/destination ids from source/destination ranks */
void optiq_path_creat_path_ids_from_path_ranks(std::vector<struct path *> &path_ids, std::vector<struct path *> &path_ranks, int num_ranks_per_node);

/* Compare 2 paths based on link load */
struct PathLinkLoadComp
{
   bool operator()(const path& p1, const path& p2)
   {
       return p1.max_load < p2.max_load;
   }
};

#endif
