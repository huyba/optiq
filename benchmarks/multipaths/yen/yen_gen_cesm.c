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

int maxpathspertest = 50 * 1024;

int maxtestid, mintestid;

void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths)
{
    int rank, size;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    char pairfile[256];

    for (int i = 0; i < jobs.size(); i++)
    {
	if (rank == i % size)
	{
	    //printf("Rank %d avail mem before call yen\n", rank);
	    //optiq_util_print_mem_info(rank);

	    optiq_alg_yen_k_shortest_paths_job (graphFilePath, jobs[i], num_paths);

	    //printf("Rank %d avail mem after call yen\n", rank);
	    //optiq_util_print_mem_info(rank);

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

void optiq_job_read_jobs_from_ca2xRearr (std::vector<struct job> &jobs, int datasize, char *cesmFilePath)
{
    char cesmfile[2048];

    for (int i = mintestid; i < maxtestid; i++)
    {
	sprintf(cesmfile, "%s/ca2xRearr.%05d", cesmFilePath, i);

	FILE * fp;
	char line[256];

	if( access( cesmfile, F_OK ) == -1 ) 
	{
	    return;
	}

	fp = fopen(cesmfile, "r");

	if (fp == NULL) {
	    return;
	}

	int job_id = 0, source_id, dest_id, num_points;

	char temp[256], name[256];
	bool exist;

	while (fgets(line, 80, fp) != NULL)
	{
	    if (line[1] == 'S')
	    {
		fgets(line, 80, fp);

		while (line[1] != 'R')
		{
		    trim(line);
		    sscanf(line, "%d %d %d", &source_id, &dest_id, &num_points);
		    /*printf("job_id = %d job_path_id = %d, flow = %f\n", job_id, job_path_id, flow);*/

		    struct job new_job;
		    new_job.job_id = job_id;
		    new_job.source_id = source_id;
		    new_job.dest_id = dest_id;
		    new_job.demand = num_points * datasize;
		    job_id++;

		    jobs.push_back(new_job);
		    
		    fgets(line, 80, fp);
		}
	    }
	}

	fclose(fp);

    }
}

void gen_patterns_cesm (struct optiq_topology *topo, int datasize, char *graphFilePath, int numpaths, char *cesmFilePath)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    int start_testid = 0;
    char jobfile[256];

    int size = topo->num_nodes * topo->num_ranks_per_node;

    jobs.clear();

    /* Subset Generate paths*/
    optiq_job_read_jobs_from_ca2xRearr (jobs, datasize, cesmFilePath);

    int maxpaths = numpaths;

    if (maxpaths > maxpathspertest/jobs.size())
    {
	maxpaths = maxpathspertest/jobs.size();
    }

    sprintf(name, "Test %d number of jobs, with %d paths per job", jobs.size(), maxpaths);
    sprintf(jobs[0].name, "%s", name);
    sprintf(jobfile, "test%d", jobs.size());

    if (rank == 0)
    {
	optiq_job_print_jobs(jobs);
    }

    //search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

    testid++;
    jobs.clear();
}       

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

    int datasize = atoi (argv[7]);

    mintestid = atoi (argv[8]);
    maxtestid = atoi (argv[9]);

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = atoi (argv[10]);

    char *cesmfilepath = argv[11];

    char graphFilePath[] = "graph";

    if (rank == 0)
    {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gen_patterns_cesm (topo, datasize, graphFilePath, numpaths, cesmfilepath);
}
