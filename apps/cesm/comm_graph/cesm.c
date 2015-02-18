#include "cesm.h"

void optiq_cesm_gen_cpl(std::vector<int> &cpl, int num_nodes)
{
    cpl.clear();

    for (int i = 0; i < num_nodes / 8 * 7; i++) {
        cpl.push_back(i);
    }
}

void optiq_cesm_gen(std::vector<int> &cpl, std::vector<int> &land, std::vector<int> &ice, std::vector<int> &ocn, std::vector<int> &atm, int num_nodes)
{
    cpl.clear();

    for (int i = 0; i < num_nodes / 8 * 7; i++) {
        cpl.push_back(i);
    }

    land.clear();

    for (int i = 0; i < num_nodes / 8 * 2; i++) {
        land.push_back(i);
    }

    ice.clear();

    for (int i = num_nodes / 8 * 2; i < num_nodes; i++) {
        ice.push_back(i);
    }

    atm.clear();

    for (int i = 0; i < num_nodes / 8 * 7; i++) {
        atm.push_back(i);
    }

    ocn.clear();

    for (int i = num_nodes / 8 * 7; i < num_nodes; i++) {
        ocn.push_back(i);
    }
}

void optiq_cesm_gen_ocn_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, num_nodes);

    for (int i = num_nodes / 8 * 7; i < num_nodes; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}

void optiq_cesm_gen_land_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, num_nodes);

    for (int i = 0; i < num_nodes / 8 * 2; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}

void optiq_cesm_gen_ice_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, int num_nodes)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, num_nodes);

    for (int i = num_nodes / 8 * 2; i < num_nodes; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}

void optiq_cesm_gen_couple(std::vector<int> sources, std::vector<int> dests, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    for (int i = 0; i < sources.size(); i++)
    {
	std::pair<int, std::vector<int> > p = std::make_pair(sources[i], dests);
        source_dests.push_back(p);
    }
}
