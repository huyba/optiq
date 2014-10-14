#include "stdio.h"
#include "stdlib.h"

#include "topology.h"

int main(int argc, char **argv)
{
    machine_type machine = BGQ;

    struct topology *topo = (struct topology*)malloc(sizeof(struct topology));

    optiq_topology_init(topo, machine);
}