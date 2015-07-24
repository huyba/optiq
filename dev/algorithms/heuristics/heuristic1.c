#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <algorithm>

#include "heuristic1.h"

void optiq_alg_heuristic1 (std::vector<struct job> &jobs, std::vector<struct path*> &paths, int maxload, int size, int num_ranks_per_node)
{
    /* If there is not loading specified, use all paths */
    if (maxload == 0) 
    {
	return;
    }

    int **load = (int **) calloc (1, sizeof(int *) * size);

    for (int i = 0; i < size; i++)
    {
	load[i] = (int *) calloc (1, sizeof(int) * size);
    }

    for (int i = 0; i < jobs.size(); i++)
    {
	jobs[i].kpaths.clear();
    }

    int cload = 1, mload;

    /* Add paths in a job that has load less than cload and update load. Add one path per job at time */
    bool added = false;
    while (cload <= maxload)
    {
        added = false;
        for (int i = 0; i < jobs.size(); i++)
        {
            if (optiq_job_add_one_path_under_load(jobs[i], cload, load))
            {
                added = true;
            }

            mload = cload;

            while (jobs[i].kpaths.size() == 0) 
            {
                mload++;

                if (optiq_job_add_one_path_under_load(jobs[i], mload, load))
                {
                    added = true;
                }
            }

            if (mload > maxload)
            {
                maxload = mload;
            }
        }

        if (!added)
        {
            cload++;
        }
    }

    /* Free remaining paths and copy from kpaths back to paths */
    paths.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            free (jobs[i].paths[j]);
        }

        jobs[i].paths.clear();

        jobs[i].paths = jobs[i].kpaths;

        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            paths.push_back(jobs[i].paths[j]);
        }
    }
}
