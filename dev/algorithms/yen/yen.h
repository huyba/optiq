#ifndef OPTIQ_YEN_H
#define OPTIQ_YEN_H

#include <vector>

void optiq_alg_yen_k_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath);

void optiq_alg_yen_k_distinct_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath);

#endif
