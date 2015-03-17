#ifndef OPTIQ_YEN_H
#define OPTIQ_YEN_H

#include <vector>

void get_yen_k_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > source_dests, int num_paths, char *graphFilePath);

#endif
