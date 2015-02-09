#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "job.h"

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
