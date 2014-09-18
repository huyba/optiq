/*hello*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <topology.h>

int main(int argc, char **argv) {
    int num_dims = 5;
    int size[5];
    int num_sources = 0, factor = 1;

    size[0] = atoi(argv[1]);
    size[1] = atoi(argv[2]);
    size[2] = atoi(argv[3]);
    size[3] = atoi(argv[4]);
    size[4] = atoi(argv[5]);

    if(argc >= 7)
	num_sources = atoi(argv[6]);
    if(argc >= 8)
        factor = atoi(argv[7]);

    int num_nodes = 1;
    for(int i = 0; i < num_dims; i++)
	num_nodes *= size[i];

    printf("set Nodes :=\n");
    for(int i = 0; i < num_nodes; i++)
	printf("%d\n", i);
    printf(";\n\n");

    printf("set Arcs :=\n");
    double cap = -1.0;
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    cap = 2048.0;
    printf("param Capacity :=\n");
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    double demand = 2048.0;
    printf("param: Jobs: Source Destination Demand :=\n");
    for(int i = 0; i < num_sources; i++) {
	for(int j = 0; j < factor; j++) {
	    printf("%d %d %d %8.1f\n", i*factor+j, i, num_nodes-num_sources*factor+i*factor+j, demand);
	}
    }
    printf(";");

    return 0;
}
