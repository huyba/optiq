#ifndef OPTIQ_ALGORITHM_H
#define OPTIQ_ALGORITHM_H

#include "manytomany.h"
#include "alltomany.h"
#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"
#include "mpiplus.h"
#include "topology.h"
#include "heuristic1.h"
#include "heuristic2.h"

/* Enumerates for search approaches */
enum search_algorithm {
    OPTIQ_ALG_NO_CONSTRAINT,
    OPTIQ_ALG_HOPS_CONSTRAINT,
    OPTIQ_ALG_HOPS_CONSTRAINT_EARLY,
    OPTIQ_ALG_KPATHS,
    OPTIQ_ALG_MODEL_PATH_BASED,
    OPTIQ_ALG_MODEL_JOB_BASED,
    OPTIQ_ALG_MPIPLUS,
    OPTIQ_ALG_HEU1,
    OPTIQ_ALG_HEU2
};

/* Search algorithm data structure */
struct optiq_algorithm {
    enum search_algorithm search_alg;
    struct multibfs *bfs;
    int num_paths_per_pair;
    int max_hops;
    int max_load;
};

extern "C" struct optiq_algorithm *algorithm;

/* Init algoirthm pointer, set defaul search algorithm */
void optiq_algorithm_init();

/* Get algorithm pointer which contains the information of current algorithm beiing used. */
struct optiq_algorithm* optiq_algorithm_get();

/* Set default algorithm to search */
void optiq_algorithm_set_search_algorithm(enum search_algorithm search_alg);

/* Free memory and other resources for the current algorithm*/
void optiq_algorithm_destroy();

/* Free memory and ready to terminate */
void optiq_algorithm_finalize();

/* Search paths for a jobs with currently selected algorithm */
void optiq_algorithm_search_path (std::vector<struct path *> &paths, std::vector<struct job> &jobs, struct multibfs *bfs, int world_rank);

#endif
