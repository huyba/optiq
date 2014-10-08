#include <stdio.h>
#include <stdlib.h>

#include <topology.h>

void main(int argc, char **argv) {

    char *filePath = "../benchmarks/data/topo_128_hopper";   

    struct topology *topo = (struct topology *)malloc(sizeof(struct topology));

    read_topology_from_file(filePath, topo);

    printf("num_dims: %d\n", topo->num_dims);
    printf("num_ranks: %d\n", topo->num_ranks);
}
