#ifndef OPTIQ_MANY_TO_MANY_H
#define OPTIQ_MANY_TO_MANY_H

#include <vector>

#include "multibfs.h"
#include "path.h"
#include "heap_path.h"

void optiq_alg_heuristic_search_manytomany_same_dests(std::vector<struct path *> &complete_paths, int num_sources, int *source_ranks, int num_dests, int *dest_ranks, struct multibfs *bfs);

void optiq_alg_heuristic_search_manytomany(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > all_sd, struct multibfs *bfs);

#endif
