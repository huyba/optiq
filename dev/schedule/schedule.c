#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <math.h>
#include <vector>
#include <mpi.h>
#include <pami.h>

#include "algorithm.h"
#include "schedule.h"

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)

struct optiq_schedule *schedule = NULL;

void optiq_schedule_init()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    if (pami_transport == NULL) {
	printf("Init pami transport first\n");
	return;
    }

    schedule = (struct optiq_schedule *) calloc (1, sizeof(struct optiq_schedule));

    schedule->world_size = pami_transport->size;
    schedule->world_rank = pami_transport->rank;

    schedule->next_dests = (int *) calloc (1, sizeof(int) * OPTIQ_MAX_NUM_PATHS);
    schedule->recv_bytes = (int *) calloc (1, sizeof(int) * schedule->world_size);

    schedule->pami_transport = pami_transport;
    pami_transport->sched = schedule;

    schedule->chunk_size = 0;

    schedule->all_num_dests = (int *) malloc (sizeof(int) * pami_transport->size);
    schedule->active_immsends = pami_transport->size;
}

struct optiq_schedule *optiq_schedule_get()
{
    return schedule;
}

void optiq_schedule_finalize()
{
    struct optiq_schedule *schedule = optiq_schedule_get();

    free(schedule->next_dests);
    free(schedule->recv_bytes);
}

void build_notify_lists(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > &notify_list, int &num_alive_flows, int world_rank)
{
    num_alive_flows = 0;
    notify_list.clear();

    for (int i = 0; i < complete_paths.size(); i++) 
    {
	for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
	{
	    if (complete_paths[i]->arcs[j].v == world_rank)
	    {
		num_alive_flows++;

		int num_vertices = complete_paths[i]->arcs.size() + 1;
		int r = ceil(log2(num_vertices));

		std::vector<int> d;
                d.clear();

		for (int q = 1; q <= r; q++) 
		{
		    if ((j+1) % (int)pow(2, q) == (num_vertices-1) % (int)pow(2,q)) {
			/*printf("Rank = %d j = %d, num_vertices = %d, r = %d, q = %d, position = %d, u = %d\n", world_rank, j, num_vertices, r, q, j + 1 - pow(2,q-1), complete_paths[i]->arcs[j + 1 - pow(2,q-1)].u);*/

			if (j + 1 - pow(2, q-1) >= 0) {
			    d.push_back(complete_paths[i]->arcs[j + 1 - pow(2,q-1)].u);
			}
		    } else {
			break;
		    }
		}

		if (d.size() > 0) 
		{
		    std::pair<int, std::vector<int> > p = make_pair(complete_paths[i]->path_id, d);
                    notify_list.push_back(p);
		}
	    }
	}
    }
}

void build_next_dests(int world_rank, int *next_dests, std::vector<struct path *> &complete_paths)
{
    memset(next_dests, 0, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

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

		struct optiq_message_header *header = pami_transport->transport_info.message_headers.back();
		pami_transport->transport_info.message_headers.pop_back();

		header->length = nbytes;
		header->source = jobs[i].source_rank;
		header->dest = jobs[i].dest_rank;
		header->path_id = jobs[i].paths[0]->path_id;

		memcpy(&header->mem, &jobs[i].send_mr, sizeof(struct optiq_memregion));
		header->mem.offset = jobs[i].send_mr.offset + jobs[i].buf_offset;
		header->original_offset = jobs[i].buf_offset;
		jobs[i].buf_offset += nbytes;

		pami_transport->transport_info.send_headers.push_back(header);

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
	    if (chunk_size == 0) {
		chunk_size = jobs[i].buf_length/2;
	    }
	    int nbytes = chunk_size;

	    if (jobs[i].buf_offset < jobs[i].buf_length) 
	    {
		if (nbytes > jobs[i].buf_length - jobs[i].buf_offset) {
		    nbytes = jobs[i].buf_length - jobs[i].buf_offset;
		}

		struct optiq_message_header *header = pami_transport->transport_info.message_headers.back();
		pami_transport->transport_info.message_headers.pop_back();

		header->length = nbytes;
		header->source = jobs[i].source_rank;
		header->dest = jobs[i].dest_rank;
		header->path_id = jobs[i].paths[jobs[i].last_path_index]->path_id;
		jobs[i].last_path_index = (jobs[i].last_path_index + 1) % jobs[i].paths.size();

		memcpy(&header->mem, &jobs[i].send_mr, sizeof(struct optiq_memregion));
		header->mem.offset = jobs[i].send_mr.offset + jobs[i].buf_offset;
		header->original_offset = jobs[i].buf_offset;
		jobs[i].buf_offset += nbytes;

		pami_transport->transport_info.send_headers.push_back(header);

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

	    /*Check if the job is already existing*/
	    bool existed = false;
	    for (int j = 0; j < schedule.local_jobs.size(); j++)
	    {
		if (schedule.local_jobs[j].dest_rank == complete_paths[i]->arcs.back().v)
		{
		    schedule.local_jobs[j].paths.push_back(complete_paths[i]);
		    existed = true;
		    break;
		}
	    }

	    if (!existed) 
	    {
		struct optiq_job new_job;

		new_job.source_rank = world_rank;
		new_job.dest_rank = complete_paths[i]->arcs.back().v;
		new_job.paths.push_back(complete_paths[i]);
		new_job.buf_offset = 0;
		new_job.last_path_index = 0;

		schedule.local_jobs.push_back(new_job);
	    }
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


int optiq_schedule_get_pair(int *sendcounts, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;
    int num_dests = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
	    num_dests++;
	}
    }

    int *dests = NULL;
    if (num_dests > 0) {
	dests = (int *) malloc (sizeof(int) * num_dests);
    }
    int j = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
            dests[j] = i;
	    j++;
        }
    }

    int *all_num_dests = pami_transport->sched->all_num_dests;

    MPI_Allgather(&num_dests, 1, MPI_INT, all_num_dests, 1, MPI_INT, MPI_COMM_WORLD);

    int offset = 0;
    int *displs = (int *)malloc (sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) 
    {
	displs[i] = offset;
	offset += all_num_dests[i];
    }

    int *all_dests = (int *) malloc (sizeof(int) * offset);

    MPI_Allgatherv(dests, num_dests, MPI_INT, all_dests, all_num_dests, displs, MPI_INT, MPI_COMM_WORLD);

    bool *distinguished_dests = (bool *) calloc (1, sizeof(bool) * world_size);
    offset = 0;

    for (int i = 0; i < world_size; i++)
    {
	if (all_num_dests[i] > 0) 
	{
	    std::vector<int> d;

	    for (int j = offset; j < offset + all_num_dests[i]; j++) 
	    {
		d.push_back (all_dests[j]);
		distinguished_dests[all_dests[j]] = true;
	    }

	    std::pair<int, std::vector<int> > p = make_pair(i, d);
	    source_dests.push_back(p);
	}
	offset += all_num_dests[i];
    }

    int num_distinguished_dests = 0;

    for (int i = 0; i < world_size; i++) 
    {
	if (distinguished_dests[i]) {
	    num_distinguished_dests++;
	}
    }
    free(distinguished_dests);

    return num_distinguished_dests;
}


void optiq_mem_reg(void *buf, int *counts, int *displs, pami_memregion_t &mr)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;

    int max_displ = 0, index = 0;
    for (int i = 0; i < world_size; i++) {
	if (max_displ < displs[i]) {
	    max_displ = displs[i];
	    index = i;
	}
    }

    int reg_size = max_displ + counts[index];
    size_t bytes = 0;

    pami_result_t result = PAMI_Memregion_create(pami_transport->context, buf, reg_size, &bytes, &mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    }
    else if (bytes < reg_size) {
	printf("Registered less\n");
    }
}

void optiq_schedule_memory_register(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls,  struct optiq_schedule *schedule)
{
    optiq_mem_reg(sendbuf, sendcounts, sdispls, schedule->send_mr.mr);
    schedule->send_mr.offset = 0;

    optiq_mem_reg(recvbuf, recvcounts, rdispls, schedule->recv_mr.mr);
    schedule->recv_mr.offset = 0;
}

/* Will do the follows:
 * 1. get the total amount of data send/recv
 * 2. create local job.
 * 3. register send/recv mem for the schedule
 * 4. collect all pairs of source-dests.
 *
 * */
void optiq_schedule_build (void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();
    int world_rank = pami_transport->rank;

    /* Gather all pairs of source-dest and demand */
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int num_jobs = optiq_schedule_get_pair (sendcounts, source_dests);

    /* Search for paths */
    std::vector<struct path *> paths;
    paths.clear();
    optiq_algorithm_search_path (paths, source_dests, bfs);
    schedule->paths = paths;

    build_next_dests(world_rank, schedule->next_dests, paths);

    int recv_len = 0, send_len = 0;

    for (int i = 0; i < pami_transport->size; i++)
    {
	recv_len += recvcounts[i];
	send_len += sendcounts[i];
    }

    if (recv_len > 0) {
	schedule->isDest = true;
    }
    if (send_len > 0) {
	schedule->isSource = true;
    }

    /* Build a schedule to transfer data */
    schedule->rdispls = rdispls;
    schedule->recv_len = recv_len;
    schedule->expecting_length = recv_len;
    schedule->remaining_jobs = num_jobs;

    /* Register memories */
    optiq_schedule_memory_register(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, schedule);

    /* Add local jobs */
    schedule->local_jobs.clear();
    for (int i = 0; i < paths.size(); i++)
    {
        if (paths[i]->arcs.front().u == world_rank) 
	{
            /*Check if the job is already existing*/
            bool existed = false;
            for (int j = 0; j < schedule->local_jobs.size(); j++)
            {
                if (schedule->local_jobs[j].dest_rank == paths[i]->arcs.back().v)
                {
                    schedule->local_jobs[j].paths.push_back (paths[i]);
                    existed = true;
                    break;
                }
            }

            if (!existed)
            {
                struct optiq_job new_job;

                new_job.source_rank = world_rank;
                new_job.dest_rank = paths[i]->arcs.back().v;
                new_job.paths.push_back(paths[i]);
                new_job.buf_offset = 0;
                new_job.last_path_index = 0;
		new_job.send_mr = schedule->send_mr;
		new_job.send_mr.offset = sdispls[new_job.dest_rank];
		new_job.buf_length = sendcounts[new_job.dest_rank];

                schedule->local_jobs.push_back(new_job);
            }
        }
    } 

    /* Split a message into chunk-size messages*/
    optiq_schedule_split_jobs_multipaths (pami_transport, schedule->local_jobs, schedule->chunk_size);

    /*Reset a few parameters*/
    optiq_schedule_set (*schedule, num_jobs, pami_transport->size);
}

/* Destroy the registered memory regions */
void optiq_schedule_destroy()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();

    schedule->isSource = false;
    schedule->isDest = false;

    memset(schedule->all_num_dests, 0, pami_transport->size * sizeof(int));
    schedule->active_immsends = pami_transport->size;
    
    optiq_schedule_mem_destroy(*schedule, pami_transport);
}
