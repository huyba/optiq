#ifndef OPTIQ_ALGORITHM_H
#define OPTIQ_ALGORITHM_H

#include "manytomany.h"
#include "alltomany.h"
#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"

enum search_algorithm {
    OPTIQ_ALG_NO_CONSTRAINT,
    OPTIQ_ALG_HOPS_CONSTRAINT,
    OPTIQ_ALG_KPATHS,
    OPTIQ_ALG_MODEL_PATH_BASED
};

struct optiq_algorithm {
    enum search_algorithm search_alg;
};

extern "C" struct optiq_algorithm *algorithm;

void optiq_algorithm_init();

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs);

#endif
