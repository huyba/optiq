#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"

void optiq_alg_heuristic_search_kpaths(std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath)
{
    complete_paths.clear();

    optiq_alg_yen_k_shortest_paths(complete_paths, jobs, num_paths, graphFilePath);
}
