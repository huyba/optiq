#ifndef OPTIQ_CESM_H
#define OPTIQ_CESM_H

#include <vector>

#include "multibfs.h"

void optiq_cesm_gen_ocn_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs);

void optiq_cesm_gen_land_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs);

void optiq_cesm_gen_ice_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs);

#endif
