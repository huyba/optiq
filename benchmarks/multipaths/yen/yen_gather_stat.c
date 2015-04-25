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
    char *path = argv[1];
    char *outpath = argv[2];

    int start = atoi (argv[2]);
    int end = atoi (argv[3]);

    int num_nodes = atoi (argv[5]);

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

        std::vector<struct job> jobs;
	jobs.clear();
        std::vector<struct path*> paths;
	paths.clear();

	optiq_jobs_read_from_file (jobs, paths, filepath);

	int totalhops = 0, totalload = 0;

	/* Get the jobs stat */
	for (int i = 0; i < jobs.size(); i++)
	{
	    for (int j = 0; j < jobs[i].paths.size(); j++)
	    {
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
	    }
	}

	outfile << fi;
	outfile << " " << maxload << " " << minload << " " << avgload << " " << medload;
	outfile << " " << maxhops << " " << minhops << " " << avghops << " " << medhops;
	outfile << std::endl;

	for (int i = 0; i < num_nodes; i++)
	{
	    memset (load[i], 0, sizeof(int) * num_nodes);
	}
	

	/* Free memory */
	for (int i = 0; i < paths.size(); i++)
	{
	    free (paths[i]);
	}
    }

    outfile.close();

    return 0;
}
