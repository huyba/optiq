#ifndef OPTIQ_YEN_H
#define OPTIQ_YEN_H

#include <vector>

void optiq_alg_yen_k_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath);

void optiq_alg_yen_k_distinct_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath, int maxload, int numnodes);

void optiq_alg_yen_k_shortest_paths_2vertices (char *graphfile, int v1, int v2, int num_paths, std::vector<struct path *> &paths);

void optiq_alg_yen_k_shortest_paths_job (char *graphfile, struct job &x, int num_paths);

void optiq_alg_yen_k_shortest_paths_jobs (char *graphfile, std::vector<struct job> &jobs, int num_paths);

#endif
