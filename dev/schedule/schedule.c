#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <math.h>
#include <vector>
#include <mpi.h>
#include <pami.h>

#include "topology.h"
#include "pathreconstruct.h"
#include "algorithm.h"
#include "schedule.h"
#include "opi.h"

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

    schedule->active_immsends = pami_transport->size;

    schedule->dmode = DQUEUE_ROUND_ROBIN;
    schedule->auto_chunksize = false;

    schedule->jobs.clear();
}

struct optiq_schedule *optiq_schedule_get()
{
    return schedule;
}

void optiq_schedule_assign_path_ids_to_jobs (std::vector<struct path *> &path_ids, std::vector<struct job> &jobs, std::vector<struct path *> &path_ranks, int ranks_per_node)
{
    int path_id = 0;

    for (int i = 0; i < path_ids.size(); i++)
    {
	for (int j = 0; j < jobs.size(); j++) 
	{
	    if (path_ids[i]->arcs.front().u == jobs[j].source_rank && path_ids[i]->arcs.back().v == jobs[j].dest_rank)
	    {
		struct path *np = (struct path*) calloc (1, sizeof(struct path));

		memcpy (np, path_ids[i], sizeof(struct path));
		np->path_id = path_id;
		np->arcs = path_ids[i]->arcs;

		path_ranks.push_back(np);

		jobs[j].paths.push_back(np);

		path_id++;
	    }
	}
    }
}

void build_notify_lists(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > &notify_list, std::vector<std::pair<int, std::vector<int> > > &intermediate_notify_list, int &num_active_paths, int world_rank)
{
    num_active_paths = 0;
    notify_list.clear();
    intermediate_notify_list.clear();
    bool isIn;

    for (int i = 0; i < complete_paths.size(); i++) 
    {
	isIn = false;
	for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
	{
	    if (complete_paths[i]->arcs[j].v == world_rank || complete_paths[i]->arcs[j].u == world_rank) {
		isIn = true;
	    }

	    if (complete_paths[i]->arcs[j].v == world_rank)
	    {
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

		    if (world_rank == complete_paths[i]->arcs.back().v) {
			notify_list.push_back(p);
		    } else {
			intermediate_notify_list.push_back(p);
		    }
		}
	    }
	}

	if (isIn) {
	    num_active_paths++;
	    isIn = false;
	}
    }
}

void build_next_dests(int world_rank, int *next_dests, std::vector<struct path *> &complete_paths)
{
    memset(next_dests, 0, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

    for (int i = 0; i < OPTIQ_MAX_NUM_PATHS; i++) {
	next_dests[i] = -1;
    }

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

void optiq_schedule_compute_assinged_len_for_path (std::vector<struct optiq_job> &jobs)
{
    for (int i = 0; i < jobs.size(); i++)
    {
	int total_flow = 0;

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    total_flow += jobs[i].paths[j]->flow;
	}

	for (int j = 0; j < jobs[i].paths.size(); j++)
        {
	    if (total_flow == 0) 
	    {
		jobs[i].paths[j]->assigned_len = jobs[i].buf_length / jobs[i].paths.size();
	    }
	    else 
            {
		jobs[i].paths[j]->assigned_len = jobs[i].buf_length * ((double)(jobs[i].paths[j]->flow) / total_flow);
	    }

	    jobs[i].paths[j]->nbytes = jobs[i].paths[j]->assigned_len;

	    /*printf("Rank %d path_id = %d flow = %d, total_flow = %d buf_len = %d, assigned_len = %d\n", jobs[i].paths[j]->arcs.front().u, jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow, total_flow, jobs[i].buf_length, jobs[i].paths[j]->assigned_len); */
	}
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
	    int nbytes = schedule->chunk_size;

	    if (schedule->auto_chunksize || schedule->chunk_size == 0) {
		nbytes = optiq_schedule_get_chunk_size (jobs[i]);
	    }

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

		/* Determine which path to use */
		bool found = false;

		while (!found)
		{
		    /*printf("Rank %d on path %d assigned_len = %d\n", jobs[i].paths[0]->arcs.front().u, jobs[i].paths[jobs[i].current_path_index]->path_id, jobs[i].paths[jobs[i].current_path_index]->assigned_len);*/
		    if (jobs[i].paths[jobs[i].current_path_index]->assigned_len > 0)
		    {
			jobs[i].paths[jobs[i].current_path_index]->assigned_len -= nbytes;
			header->path_id = jobs[i].paths[jobs[i].current_path_index]->path_id;
			found = true;

			jobs[i].paths[jobs[i].current_path_index]->copies++;
		    }

		    jobs[i].current_path_index = (jobs[i].current_path_index + 1) % jobs[i].paths.size();
		}

		memcpy(&header->mem, &jobs[i].send_mr, sizeof(struct optiq_memregion));
		header->mem.offset = jobs[i].send_mr.offset + jobs[i].buf_offset;
		header->original_offset = jobs[i].buf_offset;
		jobs[i].buf_offset += nbytes;

		pami_transport->transport_info.local_headers.push_back(header);

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

void optiq_schedule_print_notify_list(std::vector<std::pair<int, std::vector<int> > > &notify_list, int rank)
{
    for (int i = 0; i < notify_list.size(); i++)
    {
	for (int j = 0; j < notify_list[i].second.size(); j++) {
	    printf("Rank %d path_id = %d, dest = %d\n", rank, notify_list[i].first, notify_list[i].second[j]);
	}
    }
}

void optiq_schedule_set(struct optiq_schedule *schedule, int world_size)
{
    schedule->expecting_length = schedule->recv_len;
    schedule->sent_bytes = 0;
    memset (schedule->recv_bytes, 0, sizeof (int) * world_size);
}

void optiq_mem_reg (void *buf, int *counts, int *displs, pami_memregion_t *mr)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;

    int reg_size = 0, min_pivot = 0, max_pivot = 0;
    for (int i = 0; i < world_size; i++) 
    {
	if (max_pivot < counts[i] + displs[i]) {
	    max_pivot = counts[i] + displs[i];
	}
	if (min_pivot > counts[i] + displs[i]) {
	    min_pivot = counts[i] + displs[i];
	}
    }

    reg_size = max_pivot - min_pivot;

    if (reg_size > 0 && odp.print_mem_reg_msg)
    {
	if (schedule->isSource) {
	    printf("Rank %d reg_size = %d to send, min_pivot = %d, max_pivot = %d\n", pami_transport->rank, reg_size, min_pivot, max_pivot);
	}
	if (schedule->isDest) {
            printf("Rank %d reg_size = %d to recv, min_pivot = %d, max_pivot = %d\n", pami_transport->rank, reg_size, min_pivot, max_pivot);
        }
    }

    if (reg_size > 0)
    {
	size_t bytes = 0;

	pami_result_t result = PAMI_Memregion_create(pami_transport->context, &(((char *)buf)[min_pivot]), reg_size, &bytes, mr);

	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
	else if (bytes < reg_size) {
	    printf("Registered less\n");
	}
    }
}


void optiq_schedule_memory_register(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls,  struct optiq_schedule *schedule)
{
    int recv_len = 0, send_len = 0;

    for (int i = 0; i < pami_transport->size; i++)
    {
	recv_len += recvcounts[i];
	send_len += sendcounts[i];
    }

    if (recv_len > 0) 
    {
	schedule->isDest = true;
    }

    if (send_len > 0) 
    {
	schedule->isSource = true;
    }

    /* Build a schedule to transfer data */
    schedule->send_len = send_len;
    schedule->rdispls = rdispls;
    schedule->recv_len = recv_len;
    schedule->expecting_length = recv_len;

    optiq_mem_reg(sendbuf, sendcounts, sdispls, &(schedule->send_mr.mr));
    schedule->send_mr.offset = 0;

    optiq_mem_reg(recvbuf, recvcounts, rdispls, &(schedule->recv_mr.mr));
    schedule->recv_mr.offset = 0;
}

void optiq_schedule_create_local_jobs (std::vector<struct job > &jobs, std::vector<struct path *> &path_ranks, std::vector<struct optiq_job> &local_jobs, int *sendcounts, int *sdispls)
{
    local_jobs.clear();

    struct optiq_algorithm *alg = optiq_algorithm_get();
    struct optiq_schedule *schedule = optiq_schedule_get();
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_rank = pami_transport->rank;

    if (alg->search_alg == OPTIQ_ALG_MODEL_PATH_BASED)
    {
	for (int i = 0; i < jobs.size(); i++)
	{
	    if (world_rank == jobs[i].source_rank) 
	    {
		struct optiq_job new_job;
		new_job.paths.clear();

		new_job.job_id = jobs[i].job_id;
		new_job.source_rank = jobs[i].source_rank;
		new_job.dest_rank = jobs[i].dest_rank;
		new_job.paths = jobs[i].paths;
		new_job.buf_offset = 0;
		new_job.current_path_index = 0;
		memcpy(&new_job.send_mr, &schedule->send_mr, sizeof (struct optiq_memregion));
		new_job.send_mr.offset = sdispls[new_job.dest_rank];
		new_job.buf_length = sendcounts[new_job.dest_rank];

		local_jobs.push_back(new_job);
	    }
	}
    }
    else 
    {
	for (int i = 0; i < path_ranks.size(); i++)
	{
	    if (path_ranks[i]->arcs.front().u == world_rank)
	    {
		/*Check if the job is already existing*/
		bool existed = false;
		for (int j = 0; j < local_jobs.size(); j++)
		{
		    if (local_jobs[j].dest_rank == path_ranks[i]->arcs.back().v)
		    {
			local_jobs[j].paths.push_back (path_ranks[i]);
			existed = true;
			break;
		    }
		}

		if (!existed)
		{
		    struct optiq_job new_job;

		    new_job.paths.clear();

		    new_job.source_rank = world_rank;
		    new_job.dest_rank = path_ranks[i]->arcs.back().v;
		    new_job.paths.push_back(path_ranks[i]);
		    new_job.buf_offset = 0;
		    new_job.current_path_index = 0;
		    memcpy(&new_job.send_mr, &schedule->send_mr, sizeof (struct optiq_memregion));
		    new_job.send_mr.offset = sdispls[new_job.dest_rank];
		    new_job.buf_length = sendcounts[new_job.dest_rank];

		    local_jobs.push_back(new_job);
		}
	    }
	}
    }

    for (int i = 0; i < local_jobs.size(); i++)
    {
	if (local_jobs[i].paths.size() == 0)
	{
	    printf("Error ! Rank %d has no path to transfer data to %d\n", world_rank, local_jobs[i].dest_rank);
	}
    }

    schedule->maxnumpaths = 1;
    if (schedule->local_jobs.size() > 0) {
	for (int i = 0; i < schedule->local_jobs.size(); i++) {
	    if (schedule->maxnumpaths < schedule->local_jobs[i].paths.size()) {
		schedule->maxnumpaths = schedule->local_jobs[i].paths.size();
	    }
	}
    }
}

void optiq_scheduler_map_job_path_id_rank (std::vector<struct job> &jobs, std::vector<struct path *> &path_ids, std::vector<struct path *> &path_ranks)
{
    /* Map paths to from node id to rank for jobs */
    for (int i = 0; i < jobs.size(); i++)
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		jobs[i].paths[j]->arcs[k].u = jobs[i].paths[j]->arcs[k].u * topo->num_ranks_per_node + jobs[i].source_rank % topo->num_ranks_per_node;
		jobs[i].paths[j]->arcs[k].v = jobs[i].paths[j]->arcs[k].v * topo->num_ranks_per_node + jobs[i].source_rank % topo->num_ranks_per_node;
	    }

	    jobs[i].paths[j]->arcs.front().u = jobs[i].source_rank;
	    jobs[i].paths[j]->arcs.back().v = jobs[i].dest_rank;
	}
    }
}

void optiq_schedule_print_optiq_jobs (std::vector<struct optiq_job> &jobs)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_rank = pami_transport->rank;

    char strpath[1024];
    for (int i = 0; i < jobs.size(); i++)
    {
	printf("Rank %d job_id = %d source = %d dest = %d, bufsize = %d, offset = %d\n", world_rank, jobs[i].job_id, jobs[i].source_rank, jobs[i].dest_rank, jobs[i].buf_length, jobs[i].send_mr.offset);
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    printf("Rank %d job_id = %d #paths = %ld path_id = %d flow = %d\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow);

	    sprintf(strpath, "%s", "");
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		sprintf(strpath, "%s%d->", strpath, jobs[i].paths[j]->arcs[k].u);
	    }
	    sprintf(strpath, "%s%d", strpath, jobs[i].paths[j]->arcs.back().v);
	    printf("Rank %d job_id = %d #paths = %ld path_id = %d %s\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, strpath);
	}
    }
}

/* Will do the follows:
 * 1. build next_dest look-up table to look for next dest for forwarding message.
 * 2. build notify list of end nodes and intermediate nodes to nofity that a path is done.
 * 3. register send/recv mem for the schedule
 * 4. create local optiq jobs.
 * 5. split the jobs into messages.
 * 6. set recv_length for the local rank.
 * */

void optiq_scheduler_build_schedule (void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls, std::vector<struct job> &jobs, std::vector<struct path *> &path_ranks)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *sched = optiq_schedule_get();
    struct optiq_topology *topo = optiq_topology_get();

    int rank = pami_transport->rank;

    if (odp.print_path_rank && odp.print_job) {
	optiq_path_print_paths(path_ranks);
	optiq_job_print(jobs, rank);
    }

    /* Build lookup table for next dest of a forwarding message with the key is path id*/
    build_next_dests (rank, schedule->next_dests, path_ranks);

    /* Build notify list for the final dest and immediate nodes to notify path is done */
    build_notify_lists (path_ranks, schedule->notify_list, schedule->intermediate_notify_list, schedule->num_active_paths, rank);

    if (odp.print_notify_list)
    {
	optiq_schedule_print_notify_list(schedule->notify_list, rank);
	optiq_schedule_print_notify_list(schedule->intermediate_notify_list, rank);
    }

    /* Register memories */
    optiq_schedule_memory_register (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, schedule);

    /* Add local jobs */
    optiq_schedule_create_local_jobs (jobs, path_ranks, schedule->local_jobs, sendcounts, sdispls);

    if (odp.print_local_jobs) {
	optiq_schedule_print_optiq_jobs (schedule->local_jobs);
    }

    /* Assign data len for each path based on its flow value - proportional bandwidth */
    optiq_schedule_compute_assinged_len_for_path(schedule->local_jobs);

    /*printf("Rank % done done assigning len\n", rank);*/

    /* Split a message into chunk-size messages */
    optiq_schedule_split_jobs_multipaths (pami_transport, schedule->local_jobs, schedule->chunk_size);

    if (odp.collect_transport_perf)
    {
	//optiq_schedule_compute_path_hopbyte_copy_stat(schedule->local_jobs);
    }

    /*Reset a few parameters*/
    optiq_schedule_set (schedule, pami_transport->size);

    if (rank == 0 && odp.print_done_status) {
	printf("Rank %d done scheduling\n", rank);
    }
}

void optiq_schedule_compute_path_hopbyte_copy_stat(std::vector<struct optiq_job> &jobs)
{
    opi.path_hopbyte.clear();
    opi.path_copy.clear();

    std::map<int, int>::iterator it;

    for (int i = 0; i < jobs.size(); i++)
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    int hopbytes = jobs[i].paths[j]->nbytes * jobs[i].paths[j]->arcs.size();

	    it = opi.path_hopbyte.find(hopbytes);

	    if (it != opi.path_hopbyte.end())
	    {
		it->second++;
	    }
	    else
	    {
		opi.path_hopbyte.insert(std::pair<int, int> (hopbytes, 1) );
	    }

	    int copy = jobs[i].paths[j]->copies * (jobs[i].paths[j]->arcs.size() - 1);

	    it = opi.path_copy.find (copy);

	    if (it != opi.path_copy.end())
	    {
		it->second++;
	    }
	    else
	    {
		opi.path_copy.insert(std::pair<int, int> (copy, 1));
	    }
	}
    }
}

int optiq_schedule_get_chunk_size(struct optiq_job &ojob)
{

    int message_size = ojob.buf_length;
    int num_hops = ojob.paths[ojob.current_path_index]->arcs.size();
    int num_paths = ojob.paths.size();
    
    int chunk_size = message_size;

    if (num_hops < 2) {
	if (message_size < 64 * 1024) {
	    chunk_size = message_size;
	}

	if (message_size >= 64 * 1024) {
	    chunk_size = message_size/2;
	}
    }

    if (2 <= num_hops && num_hops <= 4) 
    {
	if (message_size < 16 * 1024) 
	{
	    chunk_size = message_size;
	}
	else if (message_size < 32 * 1024) 
	{
	    chunk_size = 16 * 1024;
	}
	else if (32 * 1024 <= message_size && message_size <= 64 * 1024) 
	{
	    chunk_size = 32 * 1024;
	}
	else if (message_size >= 64 * 1024) 
	{
	    if (num_paths < 3) 
	    {
                chunk_size = 16 * 1024;
            } else 
	    {
                chunk_size = 32 * 1024;
            }
	}
    }

    if (num_hops >= 5) {
	if (message_size < 64 * 1024) 
	{
	    chunk_size = message_size;
	}

	if (message_size >= 64 * 1024) 
	{
	    if (num_paths < 3) {
		chunk_size = 16 * 1024;
	    } else {
		chunk_size = 32 * 1024;
	    }
	}
    }

    return chunk_size;
}

void optiq_schedule_mem_destroy (struct optiq_schedule *schedule, struct optiq_pami_transport *pami_transport)
{
    size_t bytes;

    pami_result_t result;

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule->send_mr.mr);
    if (result != PAMI_SUCCESS)
    {
	printf("Destroy send_mr : No success\n");
    }

    result = PAMI_Memregion_destroy (pami_transport->context, &schedule->recv_mr.mr);
    if (result != PAMI_SUCCESS)
    {
	printf("Destroy recv_mr : No success\n");
    }
}

/* Destroy the registered memory regions */
void optiq_schedule_clear()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();

    schedule->num_active_paths = 0;
    schedule->notify_list.clear();

    schedule->isSource = false;
    schedule->isDest = false;

    schedule->active_immsends = pami_transport->size;

    optiq_schedule_mem_destroy (schedule, pami_transport);

    for (int i = 0; i < schedule->paths.size(); i++) {
	free(schedule->paths[i]);
    }
    schedule->jobs.clear();
    schedule->paths.clear();
    schedule->local_jobs.clear();
    schedule->notify_list.clear();
    schedule->intermediate_notify_list.clear();
}

void optiq_schedule_finalize()
{
    struct optiq_schedule *schedule = optiq_schedule_get();

    free(schedule->next_dests);
    free(schedule->recv_bytes);

    free(schedule);
}
