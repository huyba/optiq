#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <iostream>
#include <fstream>

#include "util.h"
#include "job.h"

void optiq_job_write_to_file (std::vector<struct job> &jobs, char *filepath)
{
    std::ofstream myfile;

    myfile.open (filepath);

    for (int i = 0; i < jobs.size(); i++)
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    myfile << "J " << jobs[i].job_id << " " << jobs[i].paths[j]->path_id << " " << jobs[i].paths[j]->flow << std::endl;
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		myfile << jobs[i].paths[j]->arcs[k].u << " " << jobs[i].paths[j]->arcs[k].v << std::endl;
	    }
	    myfile << std::endl;
	}
	myfile << std::endl;
    }

    myfile.close();
}

void optiq_job_print(std::vector<struct job> &jobs, int world_rank)
{
    printf("Rank %d has %ld jobs\n", world_rank, jobs.size());

    for (int i = 0; i < jobs.size(); i++)
    {
        printf("Rank %d job_id = %d source = %d dest = %d\n", world_rank, jobs[i].job_id, jobs[i].source_id, jobs[i].dest_id);
        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            printf("Rank %d job_id = %d #paths = %ld path_id = %d flow = %d\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow);
        }
    }
}

void optiq_job_map_jobs_to_source_dests (std::vector<struct job> &jobs, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
        int source_id = jobs[i].source_id;
        int dest_id = jobs[i].dest_id;
        bool found = false;

        for (int j = 0; j < source_dests.size(); j++)
        {
            if (source_dests[j].first == source_id) {
                source_dests[j].second.push_back(dest_id);
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::vector<int> dests;
            dests.clear();
            dests.push_back(dest_id);
            std::pair<int, std::vector<int> > sd = std::make_pair(source_id, dests);
            source_dests.push_back(sd);
        }
    }
}
