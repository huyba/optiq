#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <topology.h>
#include <graph.h>

int main(int argc, char **argv) {

    char *filePath = argv[1];   

    struct topology *topo = (struct topology *)malloc(sizeof(struct topology));

    optiq_read_topology_from_file(filePath, topo);

    construct_graph(*topo);
}
