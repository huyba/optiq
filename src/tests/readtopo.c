#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>

#include <topology.h>

int main(int argc, char **argv) {

    char *filePath = argv[1];   

    struct topology *topo = (struct topology *)malloc(sizeof(struct topology));

    optiq_read_topology_from_file(filePath, topo);

    printf("num_dims: %d\n", topo->num_dims);
    printf("num_ranks: %d\n", topo->num_ranks);
    printf("size: %d %d %d\n", topo->size[0], topo->size[1], topo->size[2]);

    for (int i = 0; i < topo->num_ranks; i++) {
	printf("Rank: %d nid %d coord[ %d %d %d ]\n", i, topo->all_nic_ids[i], topo->all_coords[i][0], topo->all_coords[i][1], topo->all_coords[i][2]);
    }
}
