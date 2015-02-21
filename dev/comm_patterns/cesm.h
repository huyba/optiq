#ifndef OPTIQ_CESM_H
#define OPTIQ_CESM_H

#include <vector>

void optiq_cesm_gen_ocn_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes);

void optiq_cesm_gen_land_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes);

void optiq_cesm_gen_ice_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes);

void optiq_cesm_gen(std::vector<int> &cpl, std::vector<int> &land, std::vector<int> &ice, std::vector<int> &ocn, std::vector<int> &atm, int num_nodes);

void optiq_cesm_gen_couple(std::vector<int> sources, std::vector<int> dests, std::vector<std::pair<int, std::vector<int> > > &source_dests);

#endif
