#include "stdio.h"
#include "stdlib.h"

#include "topology.h"

int main(int argc, char **argv)
{
    machine_type machine = XC30;

    struct topology *topo = (struct topology*)malloc(sizeof(struct topology));

    optiq_topology_init(topo, machine);

    char *filePath = "topo_128_edison";

    optiq_topology_get_topology_from_file(topo, filePath);

    struct topology_info *topo_info = topo->topo_info;

    struct physical_location *pl = (struct physical_location*)malloc(sizeof(struct physical_location));

    for (int i = 0; i < topo_info->num_ranks; i++) {
	optiq_topology_get_physical_location(topo, topo_info->all_coords[i], pl);
	printf("Rank %d coord [ %d %d %d ] physical location [ %d %d %d %d ]\n", i, topo_info->all_coords[i][0] ,topo_info->all_coords[i][1], topo_info->all_coords[i][2], pl->group_id, pl->cabinet_id, pl->chasis_id, pl->blade_id);
    }
}
