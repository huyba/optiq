#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yen.h"
#include <mpi.h>
#include "topology.h"
#include "job.h"
#include "patterns.h"
#include <vector>

void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths, int maxload, struct topology *topo)
{
    std::vector< struct path *> paths;

    struct timeval t0, t1;

    gettimeofday(&t0, NULL);

    optiq_alg_yen_k_distinct_shortest_paths (paths, jobs, num_paths, graphFilePath, maxload, topo->num_nodes);

    gettimeofday(&t1, NULL);

    double elapsedtime = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);

    optiq_job_remove_paths_over_maxload (jobs, 1, topo->num_nodes, topo->num_ranks_per_node);

    optiq_job_write_to_file (jobs, jobfile);

    printf("elapsed time = %8.4f (s)\n", elapsedtime/1e6);

    /*free paths*/
    for (int i = 0; i < paths.size(); i++) {
	free (paths[i]);
    }
    paths.clear();
    jobs.clear();
}

void gen_jobs_paths (struct topology *topo, int demand, char *graphFilePath, int k, int maxload)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    char jobfile[256];

    int size = topo->num_nodes;

    /* First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    if (rank == testid % numranks)
	    {
		sprintf(name, "Test No. %d: First %d ranks send data to last %d ranks", testid, m, n);
		optiq_pattern_firstm_lastn_to_jobs (jobs, size, demand, m, n);

		//optiq_job_print_jobs (jobs);

		jobs[0].name = name;
		sprintf(jobfile, "test%d", testid);
		search_and_write_to_file (jobs, jobfile, graphFilePath, k, maxload, topo);

		printf("Rank %d wrote %s\n", rank, name);
	    }
	    testid++;
	}
    }
}

int main(int argc, char **argv)
{
    MPI_Init (&argc, &argv);
    int rank;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);

    
    struct topology *topo = (struct topology *) malloc (sizeof (struct topology));
    int num_dims = 5;
    int psize[5];

    psize[0] = atoi (argv[1]);
    psize[1] = atoi (argv[2]);
    psize[2] = atoi (argv[3]);
    psize[3] = atoi (argv[4]);
    psize[4] = atoi (argv[5]);

    int k = atoi (argv[6]);

    int maxload = atoi (argv[7]);

    int demand = atoi (argv[8]) * 1024;

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = 1;

    char graphFilePath[] = "graph";

    if (rank == 0)
    {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gen_jobs_paths (topo, demand, graphFilePath, k, maxload);
}
