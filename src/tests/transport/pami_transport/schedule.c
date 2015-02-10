#include <stdio.h>
#include <stdlib.h>

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
		header->mem.offset = jobs[i].buf_offset;
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
	for (int i = 0; i < schedule.world_size; i++)
	{
	    if (schedule.recvcounts[i] != 0)
	    {
		result = PAMI_Memregion_create (pami_transport->context, &schedule.recv_buf[schedule.rdispls[i]], schedule.recvcounts[i], &bytes, &schedule.recv_memregions[i].mr);

		if (result != PAMI_SUCCESS) {
		    printf("No success\n");
		} else if (bytes < schedule.recvcounts[i]) {
		    printf("Registered less\n");
		}
	    }
	}
    }

    if (isSource) 
    {
	for (int i = 0; i < schedule.world_size; i++)
	{
	    if (schedule.sendcounts[i] != 0)
	    {
		for (int j = 0; j < schedule.local_jobs.size(); j++)
		{
		    if (schedule.local_jobs[j].dest_rank == i)
		    {
			schedule.local_jobs[j].buf_length = schedule.sendcounts[i];

			result = PAMI_Memregion_create (pami_transport->context, &schedule.send_buf[schedule.sdispls[i]], schedule.sendcounts[i], &bytes, &schedule.local_jobs[j].send_mr.mr);

			if (result != PAMI_SUCCESS) {
			    printf("No success\n");
			} else if (bytes < schedule.sendcounts[i]) {
			    printf("Registered less\n");
			}
		    }
		}
	    }
	}
    }
}
