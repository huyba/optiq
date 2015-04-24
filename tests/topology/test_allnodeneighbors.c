#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "topology.h"

void graph_gen(int num_dims, int *size)
{
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++)
    {
        num_nodes *= size[i];
    }

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
}

int main(int argc, char **argv)
{
    int num_dims = 5;
    int size[5];

    size[0] = atoi(argv[1]);
    size[1] = atoi(argv[2]);
    size[2] = atoi(argv[3]);
    size[3] = atoi(argv[4]);
    size[4] = atoi(argv[5]);

    graph_gen(num_dims, size);

    return 0;
}
