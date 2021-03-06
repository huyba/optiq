#ifndef OPTIQ_MPI_PLUS
#define OPTIQ_MPI_PLUS

#include <vector>

#include "multibfs.h"
#include "path.h"
#include "heap_path.h"

bool add_more_load (int u, int v, int **current_load, int max_load);

void optiq_alg_heuristic_search_manytomany_current_load (std::vector<struct path *> &complete_paths, std::vector<std::pair<int, int> > &source_dests, struct multibfs *bfs, int**current_load, int max_load);

void optiq_alg_heuristic_search_mpiplus (std::vector<struct path *> &paths, std::vector<std::pair<int, int> > &source_dests);

#endif
