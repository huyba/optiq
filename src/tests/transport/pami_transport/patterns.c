#include "patterns.h"

void disjoint_contigous_highratio(int num_nodes, std::vector<int> sources, std::vector<int> dests, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    for (int i = num_nodes / 8 * 7; i < num_nodes; i++) {
        dests.push_back(i);
    }

    for (int i = 0; i < num_nodes / 8 * 7; i++) {
	sources.push_back(i);
	std::pair<int, std::vector<int> > p = make_pair(i, dests);
	source_dests.push_back(p);
    }
}
