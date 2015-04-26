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
#include <fstream>

int main(int argc, char **argv)
{
    printf("hello world\n");
    char *path = argv[1];
    char *outpath = argv[2];

    int start = atoi (argv[3]);
    int end = atoi (argv[4]);

    int size[5];
    size[0] = atoi (argv[5]);
    size[1] = atoi (argv[6]);
    size[2] = atoi (argv[7]);
    size[3] = atoi (argv[8]);
    size[4] = atoi (argv[9]);

    struct optiq_topology *topo = new optiq_topology();
    optiq_topology_init_with_params(5, size, topo);

    int num_nodes = topo->num_nodes;
    int num_edges = topo->num_edges;

    printf("start %d end %d numnode %d path %s outpath %s\n", start, end, num_nodes, path, outpath);

    char filepath[256];
    std::ofstream outfile (outpath, std::ofstream::out);

    int **load = (int **) calloc (1, sizeof(int *) * num_nodes);

    for (int i = 0; i < num_nodes; i++)
    {
	load[i] = (int *) calloc (1, sizeof(int) * num_nodes);
    }

    int minload = 1000, maxload = 0, medload = 0, minhops = 1000, maxhops = 0, medhops = 0;
    double avgload, avghops;

    for (int fi = start; fi <= end; fi++)
    {
        sprintf (filepath, "%s/test%d", path, fi);
	printf("%s", filepath);

        std::vector<struct job> jobs;
	jobs.clear();
        std::vector<struct path*> paths;
	paths.clear();

	optiq_jobs_read_from_file (jobs, paths, filepath);

	printf("job size = %d, path size = %d\n", jobs.size(), paths.size());

	int totalhops = 0, totalload = 0;

	/* Get the jobs stat */
	int numpaths = 0;

	for (int i = 0; i < jobs.size(); i++)
	{
	    for (int j = 0; j < jobs[i].paths.size(); j++)
	    {
		numpaths += jobs[i].paths[j]->arcs.size();

		if (maxhops < jobs[i].paths[j]->arcs.size())
		{
		    maxhops = jobs[i].paths[j]->arcs.size();
		}

		if (minhops > jobs[i].paths[j]->arcs.size())
		{
		    minhops = jobs[i].paths[j]->arcs.size();
		}

		for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
		{
		    int u = jobs[i].paths[j]->arcs[k].u;
		    int v = jobs[i].paths[j]->arcs[k].v;
		    load[u][v]++;
		}

		totalhops += jobs[i].paths[j]->arcs.size();
	    }
	}

	int numlinkused = 0;
	for (int i = 0; i < num_nodes; i++)
	{
	    for (int j = 0; j < num_nodes; j++)
	    {
		if (maxload < load[i][j])
		{
		    maxload = load[i][j];
		}

		if (minload > load[i][j])
		{
		    minload = load[i][j];
		}

		totalload += load[i][j];

		if (load[i][j] != 0) 
		{
		    numlinkused++;
		}
	    }
	}

	if (minhops == 0) 
	{
	    minhops = 1;
	}

	if (minload == 0) {
	    minload = 1;
	}

	avghops = (double) totalhops / numpaths;

	avgload = (double)totalload / num_edges;

	double avgload1 = (double)totalload / numlinkused;

	outfile << fi;
	outfile << " " << maxload << " " << minload << " " << avgload << " " << avgload1 << " " << medload;
	outfile << " " << maxhops << " " << minhops << " " << avghops << " " << medhops;
	outfile << std::endl;

	for (int i = 0; i < num_nodes; i++)
	{
	    memset (load[i], 0, sizeof(int) * num_nodes);
	}
	

	/* Free memory */
	for (int i = 0; i < paths.size(); i++)
	{
	    delete (paths[i]);
	}
    }

    outfile.close();

    return 0;
}
