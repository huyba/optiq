#ifndef OPTIQ_ALGORITHM_H
#define OPTIQ_ALGORITHM_H

#include "manytomany.h"
#include "alltomany.h"
#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"

enum {
    OPTIQ_ALG_KPATHS,
    OPTIQ_ALG_H1
};

void optiq_algorithm_init();

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs);

#endif
