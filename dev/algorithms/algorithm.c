#include "algorithm.h"

struct optiq_algorithm *algorithm = NULL;

void optiq_algorithm_init()
{
    algorithm = (struct optiq_algorithm *) calloc (1, sizeof(struct optiq_algorithm));

    algorithm->search_alg = OPTIQ_ALG_HOPS_CONSTRAINT;

    optiq_multibfs_init();
}

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs)
{
    if (algorithm == NULL) {
	optiq_algorithm_init();
    }

    if (algorithm->search_alg == OPTIQ_ALG_HOPS_CONSTRAINT) {
	optiq_alg_heuristic_search_manytomany(paths, source_dests, bfs);
    }
}
