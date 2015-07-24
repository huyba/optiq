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
    int num_nodes = atoi (argv[4]);

    int unit = 8192;
    int demand = 8 * 1024 * 1024;

    if (argc > 5) 
    {
	unit = atoi (argv[5]);
    }

    if (argc > 6)
    {
	demand = atoi (argv[6]);
    }

    char filepath[256];

    timeval t0, t1, t2, t3;
    gettimeofday(&t0, NULL);

    for (int i = start; i <= end; i++)
    {
        if (rank == i % size)
        {
            sprintf(filepath, "%s/test%d", path, i);

            printf("Rank %d working on %s\n", rank, filepath);

            std::vector<struct job> jobs;
	    jobs.clear();
            std::vector<struct path*> paths;
	    paths.clear();

            gettimeofday(&t2, NULL);

	    optiq_job_read_from_file (jobs, paths, filepath);
            optiq_alg_heuristic2(jobs, num_nodes, unit, demand);

            gettimeofday(&t3, NULL);

            double t = (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
            printf("testid = %d, time = %8.0f\n", i, t);

            sprintf(filepath, "test%d", i);
            optiq_job_write_to_file (jobs, filepath);
        }
    }

    gettimeofday(&t1, NULL);
    double t = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);

    if (rank == 0)
    {
	printf("Time to build paths with max paths = %d to %d is %8.0f\n", start, end, t);
    }

    return 0;
}
