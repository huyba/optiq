#ifndef OPTIQ_ALGORITHM_H
#define OPTIQ_ALGORITHM_H

#include "manytomany.h"
#include "alltomany.h"
#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"
#include "mpiplus.h"

enum search_algorithm {
    OPTIQ_ALG_NO_CONSTRAINT,
    OPTIQ_ALG_HOPS_CONSTRAINT,
    OPTIQ_ALG_HOPS_CONSTRAINT_EARLY,
    OPTIQ_ALG_KPATHS,
    OPTIQ_ALG_MODEL_PATH_BASED,
    OPTIQ_ALG_MPIPLUS
};

struct optiq_algorithm {
    enum search_algorithm search_alg;
    struct multibfs *bfs;
    int num_paths_per_pair;
    int max_hops;
    int max_load;
};

extern "C" struct optiq_algorithm *algorithm;

void optiq_algorithm_init();

struct optiq_algorithm* optiq_algorithm_get();

void optiq_algorithm_set_search_algorithm(enum search_algorithm search_alg);

void optiq_algorithm_destroy();

void optiq_algorithm_finalize();

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs, int world_rank);

#endif
