#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <vector>
#include <pami.h>

#include "schedule.h"

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)

void optiq_schedule_init(struct optiq_schedule &schedule)
{
    schedule.next_dests = (int *) calloc (1, sizeof(int) * OPTIQ_MAX_NUM_PATHS);
    schedule.recv_bytes = (int *) calloc (1, sizeof(int) * schedule.world_size);

    schedule.recv_memregions = (struct optiq_memregion *) malloc (sizeof (struct optiq_memregion) * schedule.world_size);
}

void build_next_dests(int world_rank, int *next_dests, std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
        for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
        {
            if (complete_paths[i]->arcs[j].u == world_rank)
            {
                next_dests[i] = complete_paths[i]->arcs[j].v;
            }
        }
    }
}

void optiq_schedule_split_jobs (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size)
{
    bool done = false;

    while(!done)
    {
        done = true;

        for (int i = 0; i < jobs.size(); i++)
        {
            int nbytes = chunk_size;

            if (jobs[i].buf_offset < jobs[i].buf_length) 
            {
                if (nbytes > jobs[i].buf_length - jobs[i].buf_offset) {
                    nbytes = jobs[i].buf_length - jobs[i].buf_offset;
                }

                struct optiq_message_header *header = pami_transport->extra.message_headers.back();
                pami_transport->extra.message_headers.pop_back();

                header->length = nbytes;
                header->source = jobs[i].source_rank;
                header->dest = jobs[i].dest_rank;
                header->path_id = jobs[i].paths[0]->path_id;

                memcpy(&header->mem, &jobs[i].send_mr, sizeof(struct optiq_memregion));
                header->mem.offset = jobs[i].send_mr.offset + jobs[i].buf_offset;
                header->original_offset = jobs[i].buf_offset;
                jobs[i].buf_offset += nbytes;

                pami_transport->extra.send_headers.push_back(header);

                if (jobs[i].buf_offset < jobs[i].buf_length) {
                    done = false;
                }
            }
        }
    }

    /*Clean the jobs*/
    for (int i = 0; i < jobs.size(); i++)
    {
        jobs[i].buf_offset = 0;
    }
}

void optiq_schedule_create (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths)
{
    struct optiq_pami_transport *pami_transport = schedule.pami_transport;

    memset(schedule.next_dests, 0, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

    build_next_dests(schedule.world_rank, schedule.next_dests, complete_paths);

    int world_rank = schedule.world_rank;

    bool isSource = false, isDest = false;

    schedule.local_jobs.clear();

    for (int i = 0; i < complete_paths.size(); i++) 
    {
        if (complete_paths[i]->arcs.back().v == world_rank) {
            isDest = true;
        }

        if (complete_paths[i]->arcs.front().u == world_rank) {
            isSource = true;

            struct optiq_job new_job;

            new_job.source_rank = world_rank;
            new_job.dest_rank = complete_paths[i]->arcs.back().v;
            new_job.paths.push_back(complete_paths[i]);
            new_job.buf_offset = 0;

            schedule.local_jobs.push_back(new_job);
        }
    }

    schedule.isDest = isDest;

    size_t bytes;
    pami_result_t result;

    if (isDest)
    {
	size_t max_offset = 0;
        size_t min_offset = 0;
        int max_offset_index = 0;

        for (int i = 0; i < schedule.world_size; i++)
        {
            if (max_offset < schedule.rdispls[i])
            {
                max_offset = schedule.rdispls[i];
                max_offset_index = i;
            }
            if (min_offset > schedule.rdispls[i])
            {
                min_offset = schedule.rdispls[i];
            }
        }

        size_t mem_size = max_offset + (size_t)schedule.recvcounts[max_offset_index] - min_offset;

	/*printf("size_t = %d bytes, mem_size = %zu, min_offset= %zu, max_offset = %zu, count = %d,  max_offset_index = %d\n", sizeof(size_t), mem_size, min_offset, max_offset, schedule.recvcounts[max_offset_index], max_offset_index);
	*/

        result = PAMI_Memregion_create (pami_transport->context, &schedule.recv_buf[min_offset], mem_size, &bytes, &schedule.recv_mr.mr);

        if (result != PAMI_SUCCESS) 
	{
            printf("No success\n");
        } 
	else if (bytes < mem_size) 
	{
            printf("Registered less\n");
        }
    }

    if (isSource) 
    {
        int max_offset = INT_MIN;
	int min_offset = INT_MAX;
        int max_offset_index = 0;

        for (int i = 0; i < schedule.world_size; i++)
        {
            if (max_offset < schedule.sdispls[i])
            {
                max_offset = schedule.sdispls[i];
                max_offset_index = i;
            }
            if (min_offset > schedule.sdispls[i])
            {
                min_offset = schedule.sdispls[i];
            }
        }

        int mem_size = max_offset + schedule.sendcounts[max_offset_index] - min_offset;

        result = PAMI_Memregion_create (pami_transport->context, &schedule.send_buf[min_offset], mem_size, &bytes, &schedule.send_mr.mr);

        if (result != PAMI_SUCCESS) 
	{
            printf("No success\n");
        } 
	else if (bytes < mem_size) 
	{
            printf("Registered less\n");
        }

        for (int j = 0; j < schedule.local_jobs.size(); j++)
        {
            int d = schedule.local_jobs[j].dest_rank;
            schedule.local_jobs[j].buf_length = schedule.sendcounts[d];
            schedule.local_jobs[j].send_mr = schedule.send_mr;
            schedule.local_jobs[j].send_mr.offset = schedule.sdispls[d];
        }
    }
}

void optiq_schedule_print_jobs(struct optiq_schedule &schedule)
{
    std::vector<struct optiq_job> jobs = schedule.local_jobs;
    int world_rank = schedule.world_rank;

    printf("Rank %d has %ld jobs\n", world_rank, jobs.size());

    for (int i = 0; i < jobs.size(); i++)
    {
        printf("Rank %d job_id = %d source = %d dest = %d\n", world_rank, jobs[i].job_id, jobs[i].source_rank, jobs[i].dest_rank);
        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            printf("Rank %d job_id = %d #paths = %ld path_id = %d flow = %d\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow);
        }
    }
}
