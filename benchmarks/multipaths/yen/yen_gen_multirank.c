#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yen.h"
#include <mpi.h>
#include "topology.h"
#include "job.h"
#include "util.h"
#include "patterns.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>

#include <yen_gen_basic.h>

int main(int argc, char **argv)
{
    MPI_Init (&argc, &argv);
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    struct optiq_topology *topo = (struct optiq_topology *) malloc (sizeof (struct optiq_topology));
    int num_dims = 5;
    int psize[5];

    psize[0] = atoi (argv[1]);
    psize[1] = atoi (argv[2]);
    psize[2] = atoi (argv[3]);
    psize[3] = atoi (argv[4]);
    psize[4] = atoi (argv[5]);

    int numpaths = atoi (argv[6]);

    mintestid = atoi (argv[7]);
    maxtestid = atoi (argv[8]);

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = atoi (argv[9]);

    int minsize = atoi (argv[10]) * 1024;
    int maxsize = atoi (argv[11]) * 1024;
    int demand = atai (argv[12]) * 1024;

    char graphFilePath[] = "graph";

    if (rank == 0)
    {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int testid = 0;
    bool randompairing = false;
    if (argc > 13) {
        randompairing = (atoi (argv[13]) == 1);
    }

    int s1 = 0, source_node = 0;
    int s2 = 0, dest_node = 0;

    if (argc > 14)
    {
        s1 = atoi (argv[14]);
        source_nodes = atoi (argv[15]);
        s2 = atoi (argv[16]);
        dest_nodes = atoi (argv[17]);
    }

    if (argc < 14)
    {
        gen_multiranks (topo, graphFilePath, numpaths, minsize, maxsize, testid, demand, randompairing);
    }
    else
    {
        gen_multiranks2 (topo, graphFilePath, numpaths, minsize, maxsize, testid, demand, randompairing, s1, source_nodes, s2, dest_nodes);
    }
}
