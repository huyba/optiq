#include <stdio.h>
#include <stdlib.h>

#include "datagen.h"
#include "topology.h"

int main(int argc, char **argv) 
{
    int world_size = 256;
    int ratio = 64;

    printf("set Nodes :=\n");
    for (int i = 0; i < world_size; i++) {
	printf("%d\n", i);
    }

    int size[5] = {2, 4, 4, 4, 2};
    int num_dims = 5;

    printf(";\n\n");

    printf("set Arcs :=\n");
    optiq_print_arcs(num_dims, size, -1);

    printf(";\n\n");

    printf("param Capacity :=\n");
    optiq_print_arcs(num_dims, size, 2048);

    printf(";\n\n");

    int source, dest, jobId = 0, num_dests = world_size/ratio;
    float demand = 2048.0;

    printf("param: Jobs: Source Destination Demand :=\n");
    for (int i = 0; i < world_size; i ++) {
	for (int j = 0; j < num_dests; j++) {
	    source = i;
	    dest = j * ratio + ratio/2;
	    printf("%d %d %d %8.0f\n", jobId, source, dest, demand);
	    jobId++;
	}
    }
    printf(";\n\n");
}
