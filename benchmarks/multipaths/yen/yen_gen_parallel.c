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

void aggregate_paths_from_file (std::vector<struct job> &jobs, char*jobfile, struct optiq_topology *topo, int maxload)
{
    char pairfile[256];

    /* Gather data into one file */
    std::vector<struct path *> paths;
    paths.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
	sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
	optiq_jobs_read_from_file (jobs, paths, pairfile);
    }

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

void gen_patterns (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
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

		jobs.clear();
		optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, 0, n/i/r, j*n/i, topo->num_ranks_per_node, false);

		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);
		search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		if (rank == 0) 
		{
		    printf("%s\n", name);
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

		    jobs.clear();
		    optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, j*n/i, n/i/r, j*n/i + n/i - n/i/r/k, topo->num_ranks_per_node, false);

		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);
		    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		    if (rank == 0) 
		    {
			printf("%s\n", name);
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
		    sprintf (name, "Test %d: Subset %d ranks from %d to %d send data to %d rank from %d to %d", testid, n/i, j*n/i, n/i + j*n/i - 1, n/i/r, j*n/i + n/i/r/k, n/i/r + j*n/i + n/i/r/k - 1);

		    jobs.clear();
		    optiq_pattern_m_to_n_to_jobs (jobs, numranks, demand, n/i, j*n/i, n/i/r, j*n/i + n/i/r/k, topo->num_ranks_per_node, false);

		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);
		    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		    if (rank == 0) 
		    {
			printf("%s\n", name);
		    }

		    testid++;
		}
	    }
	}
    }
}


void gen_patterns_new (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
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

    /* Generate disjoint First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    if (mintestid <= testid && testid <=maxtestid) 
	    {
		optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node, false);

		/* Not allow to generate too many paths, leading to */
		int numpairs = m > n ? m : n;
		int maxpaths = numpaths;
		if (maxpathspertest / numpairs < maxpaths) {
		    maxpaths = maxpathspertest / numpairs;
		}

		sprintf(name, "Test No. %d Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, size-n, size -1, maxpaths);
		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);

		search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);
	    }

	    testid++;
	    jobs.clear();
	}
    }

    /* Overlap Generate paths */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    for (int l = m/8; l <= m/2; l *= 2)
	    {
		if (mintestid <= testid && testid <=maxtestid) 
		{
		    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, false);

		    //optiq_job_print_jobs (jobs);

		    /* Not allow to generate too many paths, leading to */
		    int numpairs = m > n ? m : n;
		    int maxpaths = numpaths;
		    if (maxpathspertest / numpairs < maxpaths) {
			maxpaths = maxpathspertest / numpairs;
		    }

		    sprintf(name, "Test No. %d Overlap %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, m-l, n + m -l -1, maxpaths);
		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);

		    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
		}
		testid++;
		jobs.clear();
	    }
	}
    }

    /* Subset Generate paths*/
    for (int m = size/8; m <= size/2; m *= 2)
    {
	for (int n = m/16; n <= m/4; n *= 2) 
	{
	    for (int p = 0; p <= m/2; p += m/4)
	    {
		if (mintestid <= testid && testid <=maxtestid) 
		{
		    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, p, topo->num_ranks_per_node, false);

		    //optiq_job_print_jobs (jobs);

		    int numpairs = m > n ? m : n;
		    int maxpaths = numpaths;
		    if (maxpathspertest / numpairs < maxpaths) {
			maxpaths = maxpathspertest / numpairs;
		    }

		    sprintf(name, "Test No. %d Subset %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, p, p+n-1, maxpaths);
		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);

		    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
		}

		testid++;
		jobs.clear();
	    }
	}
    }

    /* First 128 to last 256*/
    if (size >= 4096) 
    {
	int m = 128, n = 256;

	if (mintestid <= testid && testid <=maxtestid) 
	{
	    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size - n, topo->num_ranks_per_node, false);

	    int numpairs = m > n ? m : n;
	    int maxpaths = numpaths;
	    if (maxpathspertest / numpairs < maxpaths) {
		maxpaths = maxpathspertest / numpairs;
	    }

	    sprintf(name, "Test No. %d Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, size-n, size-1, maxpaths);
	    sprintf(jobs[0].name, "%s", name);
	    sprintf(jobfile, "test%d", testid);

	    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
	}

	testid++;
	jobs.clear();
    }

    /* Distance increase for 2k and 4k */
    if (size >= 2048 && size <= 4096)
    {
	for (int m = size/32; m <= size/2; m *= 2)
	{
	    for (int n = size/32; n <= size/2; n *= 2)
	    {
		int min = m < n ? m : n;
		int t = 0;
		if (size/min <= 8) {
		    t = 1;
		} else {
		    t = size/min/4;;
		}

                for (int d = m; d < size - n; d += min * t)
		{
		    if (mintestid <= testid && testid <=maxtestid)
		    {
			optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, d, topo->num_ranks_per_node, false);

			/* Not allow to generate too many paths, leading to */
			int numpairs = m > n ? m : n;
			int maxpaths = numpaths;
			if (maxpathspertest / numpairs < maxpaths) {
			    maxpaths = maxpathspertest / numpairs;
			}

			sprintf(name, "Test No. %d Disjoint Increasing distance %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, d, d + n - 1, maxpaths);
			sprintf(jobs[0].name, "%s", name);
			sprintf(jobfile, "test%d", testid);

			search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);
		    }

		    testid++;
		    jobs.clear();
		}
	    }
	}
    }
}

void gen_jobs_paths_new (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    int start_testid = 0;
    char jobfile[256];

    int size = topo->num_nodes;

    /* Generate disjoint First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    sprintf(name, "Test No. %d: Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, size-n, size -1);
	    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node, false);

	    sprintf(jobs[0].name, "%s", name);
	    sprintf(jobfile, "test%d", testid);
	    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

	    testid++;
	    jobs.clear();
	}
    }

    MPI_Barrier (MPI_COMM_WORLD);

    /* Collect the paths Disjoint*/
    testid = start_testid;

    int maxload = 1;

    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2)
	{
	    if (rank == testid % size) 
	    {
		sprintf(name, "Test No. %d: Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, size-n, size -1);
		optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node, false);

		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);

		maxload = 1;
		aggregate_paths_from_file (jobs, jobfile, topo, maxload);

		printf("Collected %s\n", name);
		jobs.clear();
	    }

	    testid++;
	}
    }

    start_testid = testid;

    /* Overlap Generate paths */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    for (int l = m/8; l <= m/2; l *= 2)
	    {
		optiq_util_print_mem_info(rank);

		sprintf(name, "Test No. %d: Overlap %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, m-l, n + m -l -1);
		optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, false);

		//optiq_job_print_jobs (jobs);

		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);
		search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		testid++;
		jobs.clear();
	    }
	}
    }

    MPI_Barrier (MPI_COMM_WORLD);

    /* Overlap Collect the paths */
    testid = start_testid;
    maxload = 1;

    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2)
	{
	    for (int l = m/8; l <= m/2; l *= 2)
	    {
		if (rank == testid % size) 
		{
		    optiq_util_print_mem_info(rank);

		    sprintf(name, "Test No. %d: Overlap %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, m-l, n + m -l -1);
		    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, false);
		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);

		    maxload = 1;
		    aggregate_paths_from_file (jobs, jobfile, topo, maxload);

		    printf("Collected %s\n", name);
		    jobs.clear();
		}
		testid++;
	    }
	}
    }

    start_testid = testid;

    /* Subset Generate paths*/
    for (int m = size/8; m <= size/2; m *= 2)
    {
	for (int n = m/16; n <= m/4; n *= 2) 
	{
	    for (int p = 0; p <= m/2; p += m/4)
	    {
		sprintf(name, "Test No. %d: Subset %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, p, p+n-1);
		optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, p, topo->num_ranks_per_node, false);

		//optiq_job_print_jobs (jobs);

		sprintf(jobs[0].name, "%s", name);
		sprintf(jobfile, "test%d", testid);
		search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

		testid++;
		jobs.clear();
	    }
	}
    }

    MPI_Barrier (MPI_COMM_WORLD);

    /* Subset Collect the paths */
    testid = start_testid;
    maxload = 1;

    for (int m = size/8; m <= size/2; m *= 2)
    {
	for (int n = m/16; n <= m/4; n *= 2)
	{
	    for (int p = 0;  p<= m/2; p += m/4)
	    {
		if (rank == testid % size) 
		{
		    sprintf(name, "Test No. %d: Subset %d ranks from %d to %d send data to %d ranks from %d to %d", testid, m, 0, m-1, n, p, p+n-1);
		    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, p, topo->num_ranks_per_node, false);
		    sprintf(jobs[0].name, "%s", name);
		    sprintf(jobfile, "test%d", testid);

		    maxload = 1;
		    aggregate_paths_from_file (jobs, jobfile, topo, maxload);

		    printf("Collected %s\n", name);
		    jobs.clear();
		}
		testid++;

	    }
	}
    }
}


void gen_jobs_paths (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
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
	    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

	    //printf("Rank %d wrote %s\n", rank, name);
	    testid++;
	}
    }

    MPI_Barrier (MPI_COMM_WORLD);

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

    struct optiq_topology *topo = (struct optiq_topology *) malloc (sizeof (struct optiq_topology));
    int num_dims = 5;
    int psize[5];

    psize[0] = atoi (argv[1]);
    psize[1] = atoi (argv[2]);
    psize[2] = atoi (argv[3]);
    psize[3] = atoi (argv[4]);
    psize[4] = atoi (argv[5]);

    int numpaths = atoi (argv[6]);

    int demand = atoi (argv[7]) * 1024;

    mintestid = atoi (argv[8]);
    maxtestid = atoi (argv[9]);

    optiq_topology_init_with_params(num_dims, psize, topo);
    topo->num_ranks_per_node = atoi (argv[10]);

    char graphFilePath[] = "graph";

    if (rank == 0)
    {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    gen_patterns_new (topo, demand, graphFilePath, numpaths);
    //gen_jobs_paths_new (topo, demand, graphFilePath, numpaths);
    //gen_patterns (topo, demand, graphFilePath, numpaths);
    //gen_jobs_paths (topo, demand, graphFilePath, numpaths);
}
