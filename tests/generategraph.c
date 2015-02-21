#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <topology.h>
#include <graph.h>

int main(int argc, char **argv) {

    char *filePath = argv[1];   

    struct topology *topo = (struct topology *)malloc(sizeof(struct topology));

    optiq_read_topology_from_file(filePath, topo);

    float **graph = (float **)malloc(sizeof(float *) * topo->num_ranks);

    for (int i = 0; i < topo->num_ranks; i++) {
	graph[i] = (float *)malloc(sizeof(float) * topo->num_ranks);	
    }

    construct_graph(*topo, graph);
}
