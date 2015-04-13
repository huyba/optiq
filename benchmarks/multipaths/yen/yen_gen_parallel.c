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
    int rank, size;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    char pairfile[256];

    MPI_Barrier (MPI_COMM_WORLD);

    for (int i = 0; i < jobs.size(); i++)
    {
	if (rank == i % size)
	{
	    optiq_alg_yen_k_shortest_paths_job (graphFilePath, jobs[i], num_paths);

	    sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
	    optiq_job_write_to_file (jobs, pairfile);

	    /*free paths*/
	    for (int j = 0; j < jobs[i].paths.size(); j++) 
	    {
		jobs[i].paths[j]->arcs.clear();
		free (jobs[i].paths[j]);
	    }
	    jobs[i].paths.clear();
	}
    }
}

void aggregate_paths_from_file (std::vector<struct job> &jobs, char*jobfile, struct topology *topo, int maxload)
{
    char pairfile[256];

    /*Gather data into one file*/
    std::vector<struct path *> paths;

    for (int i = 0; i < jobs.size(); i++)
    {
	sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
	optiq_jobs_read_from_file (jobs, paths, pairfile);
    }

    printf("done reading from file\n");

    optiq_job_remove_paths_over_maxload (jobs, maxload, topo->num_nodes, topo->num_ranks_per_node);

    /*Write to a file*/
    optiq_job_write_to_file (jobs, jobfile);

    for (int i = 0; i < paths.size(); i++) 
    {
	paths[i]->arcs.clear();
	free(paths[i]);
    }
    paths.clear();
}

void gen_patterns (struct topology *topo, int demand, char *graphFilePath, int numpaths)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    int testid = 0;
    char name[1024];
    char jobfile[256];

    std::vector<struct job> jobs;
    jobs.clear();

    int n = numranks;

    for (int i = 2; i <= 8; i *= 2)
    {
	for (int j = 1; j < i; j++) 
	{
	    for (int r = 1; r <= 4; r *= 2)
	    {
		sprintf (name, "Test %d: Disjoint %d ranks from %d to %d send data to %d rank from %d to %d", testid, n/i, 0, n/i-1, n/i/r, j*n/i, n/i/r + j*n/i - 1);
		optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, 0, n/i/r, j*n/i, false);

		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);
		search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		if (rank == 0) 
		{
		    printf("%s", name);
		}

		testid++;
	    }
	}
    }

    jobs.clear();

    for(int i = 2; i <= 8; i *= 2)
    {
	for (int j = 0; j < i - 1; j++)
	{
	    for (int k = 2; k <= 4; k *= 2)
	    {
		for (int r = 1; r <= 4; r *= 2)
		{
		    sprintf (name, "Test %d: Overlap %d ranks from %d to %d send data to %d rank from %d to %d", testid, n/i, j*n/i, n/i + j*n/i - 1, n/i/r , j*n/i + n/i - n/i/r/k, n/i/r + j*n/i + n/i - n/i/r/k - 1);

		    optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, j*n/i, n/i/r, j*n/i + n/i - n/i/r/k, false);

                    sprintf(jobs[0].name, "%s", name);
	            sprintf(jobfile, "test%d", testid);
	            search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		    if (rank == 0) 
		    {
			printf("%s", name);
		    }

		    testid++;
		}
	    }
	}
    }

    jobs.clear();

    for(int i = 2; i <= 8; i *= 2)
    {
	for (int j = 0; j < i; j++)
	{
	    for (int k = 2; k <= 4; k *= 2)
	    {
		for (int r = 2; r <= 4; r *= 2)
		{
		    sprintf (name, "Test %d: %d ranks from %d to %d send data to %d rank from %d to %d\n", testid, n/i, j*n/i, n/i + j*n/i - 1, n/i/r, j*n/i + n/i/r/k, n/i/r + j*n/i + n/i/r/k - 1);

		    optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, j*n/i, n/i/r, j*n/i + n/i/r/k, false);

		    sprintf(jobs[0].name, "%s", name);
                    sprintf(jobfile, "test%d", testid);
                    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		    if (rank == 0) 
		    {
			printf("%s", name);
		    }

		    testid++;
		}
	    }
	}
    }
}

void gen_jobs_paths (struct topology *topo, int demand, char *graphFilePath, int k)
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
	    sprintf(name, "Test No. %d: First %d ranks send data to last %d ranks", testid, m, n);
	    optiq_pattern_firstm_lastn_to_jobs (jobs, size, demand, m, n);

	    //optiq_job_print_jobs (jobs);

	    sprintf(jobs[0].name, "%s", name);
	    sprintf(jobfile, "test%d", testid);
	    search_and_write_to_file (jobs, jobfile, graphFilePath, k);

	    //printf("Rank %d wrote %s\n", rank, name);
	    testid++;
	}
    }

    /* Collect the paths */
    testid = 0;
    int maxload = 1;

    for (int m = size/16; m <= size/2; m *= 2)
    {
        for (int n = size/16; n <= size/2; n *= 2)
        {
	    if (testid == rank) 
	    {
		sprintf(name, "Test No. %d: First %d ranks send data to last %d ranks", testid, m, n);
		optiq_pattern_firstm_lastn_to_jobs (jobs, size, demand, m, n);
		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);

		maxload = 1;
		aggregate_paths_from_file (jobs, jobfile, topo, maxload);
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

    int numpaths = atoi (argv[6]);

    int demand = atoi (argv[7]) * 1024;

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = 1;

    char graphFilePath[] = "graph";

    if (rank == 0)
    {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gen_patterns (topo, demand, graphFilePath, numpaths);
    //gen_jobs_paths (topo, demand, graphFilePath, numpaths);
}
