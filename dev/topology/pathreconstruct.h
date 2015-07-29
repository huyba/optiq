/*
 * Used to reconstruct data movement
 * */

#ifndef OPTIQ_PATH_RECONSTRUCT_H
#define OPTIQ_PATH_RECONSTRUCT_H

#include <vector>
#include <utility>

#include "topology.h"
#include "path.h"

/* Currently used path reconstruction method to construct path for data movement using MPI */
void optiq_topology_path_reconstruct_new (std::vector<std::pair<int, int> > &source_dests, std::vector<struct path *> &mpi_paths);

/* Obsolete method for path reconstruction */
void optiq_topology_path_reconstruct (std::vector < std::pair <int, std::vector<int> > > &source_dests, struct optiq_topology *topo, std::vector<struct path *> &mpi_paths);

#endif
