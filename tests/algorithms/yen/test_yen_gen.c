#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yen.h"
#include <mpi.h>
#include "topology.h"
#include "job.h"
#include <vector>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    struct topology *topo = (struct topology *) malloc (sizeof (struct topology));
    int num_dims = 5;
    int psize[5];

    psize[0] = atoi (argv[1]);
    psize[1] = atoi (argv[2]);
    psize[2] = atoi (argv[3]);
    psize[3] = atoi (argv[4]);
    psize[4] = atoi (argv[5]);

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = 1;

    std::vector<struct path *> complete_paths;
    std::vector<struct job> jobs;
    int num_paths = atoi (argv[6]);
    char graphFilePath[] = "graph";

    if (rank == 0) {
        optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    int id = 0;
    for (int i = 0; i < topo->num_nodes / 8; i++)
    {
        struct job new_job;
        new_job.source_rank = i;
	new_job.source_id = new_job.source_rank / topo->num_ranks_per_node;
        new_job.dest_rank = topo->num_nodes * 6 / 8 + i;
	new_job.dest_id = new_job.dest_rank / topo->num_ranks_per_node;
        new_job.job_id = id;
        id++;

        jobs.push_back(new_job);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    struct timeval t0, t1;

    gettimeofday(&t0, NULL);

    optiq_alg_yen_k_distinct_shortest_paths (complete_paths, jobs, num_paths, graphFilePath);

    gettimeofday(&t1, NULL);

    double elapsedtime = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);

    if (rank == 0) {
	char jobfile[] = "jobs";
        optiq_job_write_to_file (jobs, jobfile);

	printf("elapsed time = %8.4f (s)\n", elapsedtime/1e6);
    }
}
