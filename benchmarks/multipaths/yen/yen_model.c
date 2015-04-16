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

int main(int argc, char **argv)
{
    MPI_Init (&argc, &argv);
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    char *path = argv[1];

    int start = atoi (argv[2]);
    int end = atoi (argv[3]);

    int maxload = atoi (argv[4]);
    int num_nodes = atoi (argv[5]);
    int num_ranks_per_node = atoi(argv[6]);

    int num_dims = 5;
    int size[5];
    size[0] = atoi (argv[7]);
    size[1] = atoi (argv[8]);
    size[2] = atoi (argv[9]);
    size[3] = atoi (argv[10]);
    size[4] = atoi (argv[11]);

    struct topology *topo = (struct topology*) calloc (1, sizeof(struct topology));
    optiq_topology_init_with_params (num_dims, size, topo);

    char filepath[256];
    char modeldat[256];
    int capacity = atoi (argv[12]);

    for (int i = start; i <= end; i++)
    {
        if (rank == i % numranks)
        {
            sprintf(filepath, "%s/test%d", path, i);
	    sprintf(modeldat, "model%d.dat", i);

            printf("Rank %d working on %s\n", rank, filepath);

            std::vector<struct job> jobs;
	    jobs.clear();
            std::vector<struct path*> paths;
	    paths.clear();

	    optiq_job_write_jobs_model_format(filepath, maxload, topo->num_nodes, num_ranks_per_node, topo->neighbors, capacity, modeldat);
            sprintf(filepath, "test%d", i);
            optiq_job_write_to_file (jobs, filepath);
        }
    }

    return 0;
}
