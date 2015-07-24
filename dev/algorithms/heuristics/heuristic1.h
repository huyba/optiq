#ifndef OPTIQ_ALG_HEURISTIC1_H
#define OPTIQ_ALG_HEURISTIC1_H

#include <vector>
#include "job.h"

/*
 * This is heuristic 1 for data movement between a set of sources and destinations.
 *
 * Arguments:
 * jobs: set of jobs with sources and destinations, demand is not required.
 * paths: output variable, also contains the set of paths that will be used.
 * maxload: maximum number of paths per physical links.
 * size: number of nodes in the given partition.
 * num_ranks_per_node: number of ranks per node.
 * */
void optiq_alg_heuristic1 (std::vector<struct job> &jobs, std::vector<struct path*> &paths, int maxload, int size, int num_ranks_per_node);

#endif
