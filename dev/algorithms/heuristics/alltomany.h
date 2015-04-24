#ifndef OPTIQ_ALL_TO_MANY_H
#define OPTIQ_ALL_TO_MANY_H

#include <vector>

#include "multibfs.h"
#include "path.h"
#include "heap_path.h"

void optiq_alg_heuristic_search_alltomany(std::vector<struct path *> &complete_paths, int num_dests, int *dests, struct multibfs *bfs);

#endif
