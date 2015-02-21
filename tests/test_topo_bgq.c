#include "stdio.h"
#include "stdlib.h"

#include "topology.h"

int main(int argc, char **argv)
{
    machine_type machine = BGQ;

    struct optiq_topology *topo = (struct optiq_topology*)malloc(sizeof(struct optiq_topology));

    optiq_topology_init(topo, machine);
}
