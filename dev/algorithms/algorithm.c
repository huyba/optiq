#include "algorithm.h"

void optiq_algorithm_init()
{

}

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs)
{
    optiq_alg_heuristic_search_manytomany(paths, source_dests, bfs);
}
