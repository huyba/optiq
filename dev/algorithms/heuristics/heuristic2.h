#ifndef OPTIQ_ALG_HEURISTIC2_H
#define OPTIQ_ALG_HEURISTIC2_H

#include <vector>
#include "job.h"

/*
 * This is heuristic 2 algorithm for compute paths between a set of nodes. Each job has a source, destination. It may also have demand, if not the demand argument will be used.
 * Arguments:
 * jobs: set of jobs with source, destination, demand.
 * num_nodes: number of nodes of given partition.
 * unit: the data size that is given for each path at each iteration.
 * demand: will be used if the job's demand is not used.
 * */

void optiq_alg_heuristic2 (std::vector<struct job> &jobs, int num_nodes, int unit, int demand);

#endif
