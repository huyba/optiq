#include "cesm.h"

void optiq_cesm_gen_cpl(std::vector<int> &cpl, struct multibfs &bfs)
{
    cpl.clear();

    for (int i = 0; i < bfs.num_nodes / 8 * 7; i++) {
        cpl.push_back(i);
    }
}

void optiq_cesm_gen_ocn_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, bfs);

    for (int i = bfs.num_nodes / 8 * 7; i < bfs.num_nodes; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}

void optiq_cesm_gen_land_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, bfs);

    for (int i = 0; i < bfs.num_nodes / 8 * 2; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}

void optiq_cesm_gen_ice_cpl(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct multibfs &bfs)
{
    source_dests.clear();

    std::vector<int> cpl;
    optiq_cesm_gen_cpl(cpl, bfs);

    for (int i = bfs.num_nodes / 8 * 2; i < bfs.num_nodes; i++) {
        std::pair<int, std::vector<int> > p = std::make_pair(i, cpl);
        source_dests.push_back(p);
    }
}
