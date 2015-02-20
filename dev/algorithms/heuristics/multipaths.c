#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "multibfs.h"
#include "multipaths.h"
#include "yen.h"

void optiq_graph_print_graph(struct multibfs &bfs, int cost, char *filePath)
{
    std::ofstream myfile;
    myfile.open (filePath);
    myfile << bfs.num_nodes << "\n\n";

    for (int i = 0; i < bfs.num_nodes; i++)
    {
	for (int j = 0; j < bfs.neighbors[i].size(); j++)
	{
	    myfile << i << " " << bfs.neighbors[i][j] << " " << cost << "\n";
	}
    }

    myfile.close();
}

void optiq_alg_heuristic_search_kpaths(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > source_dests, int num_paths, char *graphFilePath)
{
    get_yen_k_shortest_paths(complete_paths, source_dests, num_paths, graphFilePath);
}
