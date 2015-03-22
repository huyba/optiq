#include <stdlib.h>

#include "search_util.h"

bool check_and_reverse(std::vector<std::pair<int, std::vector<int> > > &all_sd, std::vector<std::pair<int, std::vector<int> > > &sd, int num_nodes)
{
    bool isReverted = false;

    int *source_dest = (int *) calloc (1, sizeof(int) * num_nodes * num_nodes);

    for (int i = 0; i < all_sd.size(); i++) {
        for (int j = 0; j < all_sd[i].second.size(); j++) {
            source_dest[all_sd[i].first * num_nodes + all_sd[i].second[j]] = 1;
        }
    }

    int num_dests = 0;
    bool is_dest;
    for (int j = 0; j < num_nodes; j++)
    {
        is_dest = false;

        for (int i = 0; i < num_nodes; i++)
        {
            if (source_dest[i * num_nodes + j] != 0) {
                is_dest = true;
                break;
            }
        }

        if (is_dest) {
            num_dests++;
        }
    }

    int num_sources = all_sd.size();

    if (num_dests < num_sources)
    {
	isReverted = true;

        for (int j = 0; j < num_nodes; j++)
        {
            std::vector<int> sources;
            sources.clear();

            for (int i = 0; i < num_nodes; i++)
            {
                if (source_dest[i * num_nodes + j] != 0) {
                    sources.push_back(i);
                }
            }

            if (sources.size() > 0) {
                std::pair<int, std::vector<int> > p = std::make_pair (j, sources);
                sd.push_back(p);
            }
        }
    }

    free(source_dest);

    return isReverted;
}
