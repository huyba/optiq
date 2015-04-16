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
    int rank, size;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);

    char *path = argv[1];

    int start = atoi (argv[2]);
    int end = atoi (argv[3]);

    int maxload = atoi (argv[4]);
    int num_nodes = atoi (argv[5]);
    int num_ranks_per_node = atoi(argv[6]);

    char filepath[256];

    for (int i = start; i <= end; i++)
    {
        if (rank == i % size)
        {
            sprintf(filepath, "%s/test%d", path, i);

            printf("Rank %d working on %s\n", rank, filepath);

            std::vector<struct job> jobs;
            std::vector<struct path*> paths;

            optiq_job_read_and_select(jobs, paths, filepath, maxload, num_nodes, num_ranks_per_node);
            sprintf(filepath, "test%d", i);
            optiq_job_write_to_file (jobs, filepath);
        }
    }

    return 0;
}
