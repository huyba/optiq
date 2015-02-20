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

void optiq_schedule_finalize(struct optiq_schedule &schedule)
{
    free(schedule.next_dests);
    free(schedule.recv_bytes);
    free(schedule.recv_memregions);
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

void optiq_schedule_split_jobs_multipaths (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size)
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
                header->path_id = jobs[i].paths[jobs[i].last_path_index]->path_id;
		jobs[i].last_path_index = (jobs[i].last_path_index + 1) % jobs[i].paths.size();

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

void optiq_schedule_add_paths (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths)
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
	    new_job.last_path_index = 0;

            schedule.local_jobs.push_back(new_job);
        }
    }

    schedule.isDest = isDest;
    schedule.isSource = isSource;
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


void optiq_schedule_mem_destroy(struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport)
{
    size_t bytes;

    pami_result_t result;

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule.send_mr.mr);
    if (result != PAMI_SUCCESS)
    {
	printf("Destroy send_mr : No success\n");
    }

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule.recv_mr.mr);
    if (result != PAMI_SUCCESS)
    {
	printf("Destroy recv_mr : No success\n");
    }
}

/* Register memory for sending and receiving and assign mem region for local_jobs*/
void optiq_schedule_mem_reg (struct optiq_schedule &schedule, struct optiq_comm_mem &comm_mem, struct optiq_pami_transport *pami_transport)
{
    schedule.rdispls = comm_mem.rdispls;

    size_t bytes;

    pami_result_t result;

    /* Register memory for sending data */
    if (comm_mem.send_len > 0) 
    {
	result = PAMI_Memregion_create (pami_transport->context, comm_mem.send_buf, comm_mem.send_len, &bytes, &schedule.send_mr.mr);
	schedule.send_mr.offset = 0;

	if (result != PAMI_SUCCESS)
	{
	    printf("No success\n");
	}
	else if (bytes < comm_mem.send_len)
	{
	    printf("Registered less\n");
	}
    }

    /* Assign mem region for local jobs */
    int dest_rank;
    for (int i = 0; i < schedule.local_jobs.size(); i++)
    {
	dest_rank = schedule.local_jobs[i].dest_rank;
	schedule.local_jobs[i].send_mr = schedule.send_mr;
	schedule.local_jobs[i].send_mr.offset = comm_mem.sdispls[dest_rank];
    }

    /* Create memory region for receiving data*/
    if (comm_mem.recv_len > 0)
    {
	result = PAMI_Memregion_create (pami_transport->context, comm_mem.recv_buf, comm_mem.recv_len, &bytes, &schedule.recv_mr.mr);
	schedule.recv_mr.offset = 0;

	if (result != PAMI_SUCCESS)
	{
	    printf("No success\n");
	}
	else if (bytes < comm_mem.recv_len)
	{
	    printf("Registered less\n");
	}
    }
}

void optiq_schedule_set(struct optiq_schedule &schedule, int num_jobs, int world_size)
{
    schedule.remaining_jobs = num_jobs;
    schedule.expecting_length = schedule.recv_len;
    schedule.sent_bytes = 0;
    memset (schedule.recv_bytes, 0, sizeof (int) * world_size);
}

void optiq_schedule_assign_job_demand(std::vector<struct optiq_job> &local_jobs, int nbytes)
{
    for (int i = 0; i < local_jobs.size(); i++) {
        local_jobs[i].buf_length = nbytes;
    }
}
