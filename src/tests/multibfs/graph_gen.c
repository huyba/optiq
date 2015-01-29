#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "topology.h"

int main(int argc, char **argv)
{
    int num_dims = 5;
    int size[5] = {2,4,4,4,2};
    int num_nodes = 256;

    std::vector<int> *neighbors = optiq_topology_get_all_nodes_neighbors(num_dims, size);

    printf("%d\n\n", num_nodes);

    int cost = 1;
    for (int i = 0; i < num_nodes; i++)
    {
	for (int j = 0; j < neighbors[i].size(); j++)
	{
	    printf("%d %d %d\n", i, neighbors[i][j], cost);
	}
    }

    return 0;
}
