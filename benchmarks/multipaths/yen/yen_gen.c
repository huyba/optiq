#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "yen.h"
#include <mpi.h>
#include "topology.h"
#include "job.h"
#include "patterns.h"
#include <vector>

void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths)
{
    std::vector< struct path *> paths;

    struct timeval t0, t1;

    gettimeofday(&t0, NULL);

    optiq_alg_yen_k_distinct_shortest_paths (paths, jobs, num_paths, graphFilePath);

    gettimeofday(&t1, NULL);

    double elapsedtime = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);

    optiq_job_write_to_file (jobs, jobfile);

    printf("elapsed time = %8.4f (s)\n", elapsedtime/1e6);

    /*free paths*/
    for (int i = 0; i < paths.size(); i++) {
	free (paths[i]);
    }
    paths.clear();
    jobs.clear();
}

void gen_jobs_paths (int size, int demand, char *graphFilePath)
{
    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    char jobfile[256];
    int k = 3;

    /* First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    sprintf(name, "Test No. %d: First %d ranks send data to last %d ranks", testid, m, n);
	    optiq_pattern_firstm_lastn_to_jobs (jobs, size, demand, m, n);
	    jobs[0].name = name;
	    sprintf(jobfile, "test%d", testid);
	    search_and_write_to_file (jobs, jobfile, graphFilePath, k);
	    testid++;
	}
    }
}

int main(int argc, char **argv)
{
    struct topology *topo = (struct topology *) malloc (sizeof (struct topology));
    int num_dims = 5;
    int psize[5];

    psize[0] = atoi (argv[1]);
    psize[1] = atoi (argv[2]);
    psize[2] = atoi (argv[3]);
    psize[3] = atoi (argv[4]);
    psize[4] = atoi (argv[5]);

    int demand = atoi (argv[6]);

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = 1;

    char graphFilePath[] = "graph";
    optiq_topology_write_graph (topo, 1, graphFilePath);

    gen_jobs_paths (topo->num_nodes, demand, graphFilePath);
}
