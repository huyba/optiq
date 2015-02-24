#ifndef OPTIQ_PATH_RECONSTRUCT_H
#define OPTIQ_PATH_RECONSTRUCT_H

#include <vector>
#include <utility>

#include "topology.h"
#include "path.h"

void optiq_topology_path_reconstruct (std::vector < std::pair <int, std::vector<int> > > &source_dests, struct topology *topo, std::vector<struct path *> &mpi_paths);

#endif
