#ifndef OPTIQ_MULTIPATHS
#define OPTIQ_MULTIPATHS

#include <vector>

#include <stdio.h>
#include <stdlib.h>

void optiq_graph_print_graph(struct multibfs *bfs, int cost, char *filePath);

void optiq_alg_heuristic_search_kpaths(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > source_dests, int num_paths, char *graphFilePath);

#endif
