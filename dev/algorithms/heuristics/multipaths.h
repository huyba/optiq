#ifndef OPTIQ_MULTIPATHS
#define OPTIQ_MULTIPATHS

#include <vector>

#include <stdio.h>
#include <stdlib.h>

void optiq_alg_heuristic_search_kpaths(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs, int num_paths, char *graphFilePath);

#endif
