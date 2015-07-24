#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <algorithm>

#include "heap_path.h"
#include "heuristic2.h"

void optiq_alg_heuristic2 (std::vector<struct job> &jobs, int num_nodes, int unit, int demand)
{
    /* Load of links */
    int **load = (int **) calloc (1, sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
	load[i] = (int *) calloc (1, sizeof(int) * num_nodes);
    }

    /* Paths that use links */
    std::vector<struct path*> **link_paths = (std::vector<struct path*> **) calloc (1, sizeof(std::vector<struct path*> *) * num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
        link_paths[i] = (std::vector<struct path*> *) calloc (1, sizeof(std::vector<struct path*>) * num_nodes);
    }

    /* Init the list of paths that use links */
    for (int i = 0; i < jobs.size(); i++)
    {
	if (jobs[i].demand <= 0)
	{
	    jobs[i].demand = demand;
	}

        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            jobs[i].paths[j]->assigned_len = 0;
            jobs[i].paths[j]->flow = 0;
            for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
            {
                int u = jobs[i].paths[j]->arcs[k].u;
                int v = jobs[i].paths[j]->arcs[k].v;

                link_paths[u][v].push_back(jobs[i].paths[j]);
            }
        }
    }

    std::vector<struct job> temp_jobs = jobs;

    jobs.clear();

    /* Make heap of job by the demand */
    std::make_heap(temp_jobs.begin(), temp_jobs.end(), JobDemandComp());


    while (temp_jobs.size() > 0)
    {
        /* The the first job which has the max demand*/
        struct job tj = temp_jobs[0];

        if (tj.demand < unit) {
            unit = tj.demand;
        }

        /* Look for path with the minimum max_load */
        int max_load = INT_MAX, index = 0;
        for (int i = 0; i < tj.paths.size(); i++)
        {
            if (tj.paths[i]->max_load < max_load)
            {
                max_load = tj.paths[i]->max_load;
                index = i;
            }
        }

        /*printf("job %d %d %d %d %d %d %d %d\n", tj.job_id, tj.paths[index]->path_id, tj.paths[index]->flow, tj.source_id, tj.dest_id, tj.source_rank, tj.dest_rank, tj.demand);*/

        tj.demand -= unit;
        tj.paths[index]->assigned_len += unit;
        tj.paths[index]->flow ++;

        /* Updated the link load and paths of jobs */
        for (int i = 0; i < tj.paths[index]->arcs.size(); i++)
        {
            int u = tj.paths[index]->arcs[i].u;
            int v = tj.paths[index]->arcs[i].v;

            load[u][v] += unit;

            for (int j = 0; j < link_paths[u][v].size(); j++)
            {
                if (link_paths[u][v][j]->max_load < load[u][v])
                {
                    link_paths[u][v][j]->max_load = load[u][v];
                }
            }
        }

        /* Update heap of jobs */
        std::pop_heap (temp_jobs.begin(), temp_jobs.end(),  JobDemandComp());
        temp_jobs.pop_back();

        if (tj.demand <= 0)
        {
            for (int i = 0; i < tj.paths.size(); i++)
            {
                tj.demand += tj.paths[i]->assigned_len;

                if (tj.paths[i]->flow == 0)
                {
                    tj.paths.erase(tj.paths.begin() + i);
                    i--;
                }
            }

            printf("job %d %d %d %d %d %d %d %d\n", tj.job_id, tj.paths[index]->path_id, tj.paths[index]->flow, tj.source_id, tj.dest_id, tj.source_rank, tj.dest_rank, tj.demand);
            jobs.push_back(tj);
        }
        else
        {
            temp_jobs.push_back(tj);
            std::push_heap (temp_jobs.begin(), temp_jobs.end(),  JobDemandComp());
        }
    }
}
