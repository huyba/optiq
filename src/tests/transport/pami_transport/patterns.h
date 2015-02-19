#ifndef OPTIQ_PATTERNS_H
#define OPTIQ_PATTERNS_H

#include <vector>
#include <utility> 

void disjoint_contigous_highratio(int num_nodes, std::vector<int> sources, std::vector<int> dests, std::vector<std::pair<int, std::vector<int> > > &source_dests);

#endif
