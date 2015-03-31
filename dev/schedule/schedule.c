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

    schedule->all_num_dests = (int *) malloc (sizeof(int) * pami_transport->size);
    schedule->active_immsends = pami_transport->size;

    schedule->dmode = DQUEUE_ROUND_ROBIN;
    schedule->auto_chunksize = false;

    schedule->jobs.clear();
}

struct optiq_schedule *optiq_schedule_get()
{
    return schedule;
}

void optiq_schedule_set_dqueue_mode(enum dequeue_mode dmode)
{
    struct optiq_schedule *schedule = optiq_schedule_get();
    schedule->dmode = dmode;
}

void optiq_schedule_finalize()
{
    struct optiq_schedule *schedule = optiq_schedule_get();

    free(schedule->next_dests);
    free(schedule->recv_bytes);
    free(schedule->all_num_dests);

    free(schedule);
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

void optiq_schedule_split_jobs (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size)
{
    bool done = false;

    while(!done)
    {
        done = true;

        for (int i = 0; i < jobs.size(); i++)
        {
            int num_hops = optiq_topology_get_hop_distance (jobs[i].source_rank, jobs[i].dest_rank);

            if (schedule->auto_chunksize || schedule->chunk_size == 0) {
                schedule->chunk_size = optiq_schedule_get_chunk_size (jobs[i].buf_length, num_hops);
            }

            int nbytes = schedule->chunk_size;

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
            int nbytes = schedule->chunk_size;

            if (schedule->auto_chunksize || schedule->chunk_size == 0) {
                nbytes = optiq_schedule_get_chunk_size (jobs[i].buf_length, jobs[i].paths[jobs[i].last_path_index]->arcs.size());
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

void optiq_schedule_print_notify_list(std::vector<std::pair<int, std::vector<int> > > &notify_list, int rank)
{
    for (int i = 0; i < notify_list.size(); i++)
    {
        for (int j = 0; j < notify_list[i].second.size(); j++) {
            printf("Rank %d path_id = %d, dest = %d\n", rank, notify_list[i].first, notify_list[i].second[j]);
        }
    }
}

void optiq_schedule_print_jobs (std::vector<struct optiq_job> jobs)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_rank = pami_transport->rank;

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


int optiq_schedule_get_pair(int *sendcounts, std::vector<std::pair<int, std::vector<int> > > &source_dests, std::vector<struct job> *jobs)
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

    int id = 0;
    for (int i = 0; i < world_size; i++)
    {
        if (all_num_dests[i] > 0) 
        {
            std::vector<int> d;

            for (int j = offset; j < offset + all_num_dests[i]; j++) 
            {
                d.push_back (all_dests[j]);
                distinguished_dests[all_dests[j]] = true;

		if (jobs != NULL)
		{
		    struct job newjob;
		    newjob.source_id = i;	
		    newjob.dest_id = all_dests[j];
		    newjob.job_id = id;

		    jobs->push_back(newjob);

		    id++;
		}
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

    free (distinguished_dests);
    free (all_dests);

    if (dests != NULL) {
        free(dests);
    }

    free(displs);

    return num_distinguished_dests;
}


void optiq_mem_reg(void *buf, int *counts, int *displs, pami_memregion_t *mr)
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
    /*if (schedule->isSource || schedule->isDest) {
      printf("Rank %d reg_size = %d, min_pivot = %d, max_pivot = %d\n", pami_transport->rank, reg_size, min_pivot, max_pivot);
      }*/

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
            if (world_rank == jobs[i].source_id) 
            {
                struct optiq_job new_job;

                new_job.source_rank = jobs[i].source_id;
                new_job.dest_rank = jobs[i].dest_id;
                new_job.paths = jobs[i].paths;
                new_job.buf_offset = 0;
                new_job.last_path_index = 0;
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

                    new_job.source_rank = world_rank;
                    new_job.dest_rank = path_ranks[i]->arcs.back().v;
                    new_job.paths.push_back(path_ranks[i]);
                    new_job.buf_offset = 0;
                    new_job.last_path_index = 0;
                    memcpy(&new_job.send_mr, &schedule->send_mr, sizeof (struct optiq_memregion));
                    new_job.send_mr.offset = sdispls[new_job.dest_rank];
                    new_job.buf_length = sendcounts[new_job.dest_rank];

                    local_jobs.push_back(new_job);
                }
            }
        }
    }
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
    struct optiq_algorithm *algorithm = optiq_algorithm_get();
    struct topology *topo = optiq_topology_get();

    int world_rank = pami_transport->rank;

    /* Gather all pairs of source-dest and demand */
    std::vector<std::pair<int, std::vector<int> > > source_dest_ranks;
    int num_jobs = 0;

    if (schedule->jobs.size() == 0) {
	num_jobs = optiq_schedule_get_pair (sendcounts, source_dest_ranks, &(schedule->jobs));
    }

    /*if (world_rank == 0) {
      printf ("Done getting %d pairs of ranks\n", source_dest_ranks.size());
      }*/

    std::vector<std::pair<int, std::vector<int> > > source_dest_ids;
    source_dest_ids.clear();

    /* Convert from pair of ranks to pairs of node ids*/
    if (topo->num_ranks_per_node > 1) {
        optiq_schedule_map_from_rankpairs_to_idpairs(source_dest_ranks, source_dest_ids);
    } else {
        source_dest_ids = source_dest_ranks;
    }

    /*if (world_rank == 0) {
      printf("Done mapping pairs: %d pairs of ranks -> %d pairs of node ids\n", source_dest_ranks.size(), source_dest_ids.size());
      optiq_util_print_source_dests(source_dest_ids);
      }*/

    /* Search for paths */
    std::vector<struct path *> path_ids;
    path_ids.clear();
    if (algorithm->max_hops == 0) {
        algorithm->max_hops = optiq_topology_max_distance_2sets_with_torus (source_dest_ids);
    }

    optiq_algorithm_search_path (path_ids, schedule->jobs, bfs, world_rank);

    if (world_rank == 0 && odp.print_path_id) 
    {
        printf("Done searching %d paths of node ids\n", path_ids.size());
        optiq_path_print_paths_coords (path_ids, topo->all_coords);

        optiq_job_print (schedule->jobs, 0);
    }

    /*std::vector<struct path *> reduced_paths;
      optiq_topology_reduce_intermediate_nodes (path_ids, reduced_paths);

      if (world_rank == 0 && odp.print_reduced_paths) {
      printf("Done reducing %d paths of node ids\n", reduced_paths.size());
      optiq_path_print_paths_coords (reduced_paths, topo->all_coords);
      }
      */

    struct optiq_performance_index *opi = optiq_opi_get();
    opi->paths = path_ids;

    /* Convert from path of node ids to path of rank ids */
    std::vector<struct path *> &path_ranks = schedule->paths;
    path_ranks.clear();

    optiq_schedule_map_from_pathids_to_pathranks (path_ids, source_dest_ranks, path_ranks);

    if (world_rank == 0 && odp.print_path_rank) 
    {
        printf("Done mapping paths of node ids to ranks\n");
        printf("Num of paths = %d\n", path_ranks.size());
        optiq_path_print_paths (path_ranks);
    }

    build_next_dests(world_rank, schedule->next_dests, path_ranks);

    /*if (world_rank == 0) {
      printf("Done building next dests\n");
      }*/

    build_notify_lists(path_ranks, schedule->notify_list, schedule->intermediate_notify_list, schedule->num_active_paths, world_rank);
    /*optiq_schedule_print_notify_list(schedule->notify_list, world_rank);*/
    /*if (world_rank == 0) {
      printf("Done building notify list\n");
      }*/

    /*optiq_path_print_paths(paths);
      printf("active = %d\n", schedule->num_active_paths);
      */

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

    /*printf("Rank %d expecting_len = %d, num_active_paths = %d\n", world_rank, schedule->expecting_length, schedule->num_active_paths);*/

    /* Register memories */
    optiq_schedule_memory_register(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls, schedule);

    /* Add local jobs */
    optiq_schedule_create_local_jobs (schedule->jobs, path_ranks, schedule->local_jobs, sendcounts, sdispls);

    //optiq_schedule_print_jobs (schedule->local_jobs);

    schedule->maxnumpaths = 1;
    if (schedule->local_jobs.size() > 0) {
        for (int i = 0; i < schedule->local_jobs.size(); i++) {
            if (schedule->maxnumpaths < schedule->local_jobs[i].paths.size()) {
                schedule->maxnumpaths = schedule->local_jobs[i].paths.size();
            }
        }
    }


    /*for (int i = 0; i < schedule->local_jobs.size(); i++)
      {
      printf("Rank %d numpaths = %d\n", world_rank, schedule->local_jobs[i].paths.size());
      }*/

    /* Split a message into chunk-size messages*/
    optiq_schedule_split_jobs_multipaths (pami_transport, schedule->local_jobs, schedule->chunk_size);

    /*Reset a few parameters*/
    optiq_schedule_set (*schedule, num_jobs, pami_transport->size);

    /*Free path_ids*/
    /*optiq_algorithm_destroy();*/
}

/* Destroy the registered memory regions */
void optiq_schedule_destroy()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();

    schedule->num_active_paths = 0;
    schedule->notify_list.clear();

    schedule->isSource = false;
    schedule->isDest = false;

    memset(schedule->all_num_dests, 0, pami_transport->size * sizeof(int));
    schedule->active_immsends = pami_transport->size;

    optiq_schedule_mem_destroy(*schedule, pami_transport);

    for (int i = 0; i < schedule->paths.size(); i++) {
        free(schedule->paths[i]);
    }
    schedule->paths.clear();
}

int optiq_schedule_get_chunk_size(int message_size, int num_hops) 
{
    int chunk_size = message_size;

    if (num_hops < 2) {
        if (message_size < 64 * 1024) {
            chunk_size = message_size;
        }

        if (message_size >= 64 * 1024) {
            chunk_size = message_size/2;
        }
    }

    if (2 <= num_hops && num_hops <= 4) {
        if (message_size < 16 * 1024) {
            chunk_size = message_size;
        }
        else if (message_size < 32 * 1024) {
            chunk_size = 16 * 1024;
        }
        else if (32 * 1024 <= message_size && message_size <= 64 * 1024) {
            chunk_size = 32 * 1024;
        }
        else if (message_size >= 64 * 1024) {
            chunk_size = 16 * 1024;
        }
    }

    if (num_hops >= 5) {
        if (message_size < 64 * 1024) {
            chunk_size = message_size;
        }

        if (message_size >= 64 * 1024) {
            chunk_size = 16 * 1024;
        }
    }

    return chunk_size;
}

void optiq_schedule_map_from_rankpairs_to_idpairs(std::vector<std::pair<int, std::vector<int> > > &source_dest_ranks, std::vector<std::pair<int, std::vector<int> > > &source_dest_ids)
{
    source_dest_ids.clear();

    for (int i = 0; i < source_dest_ranks.size(); i++) 
    {
        int source_id = optiq_topology_get_node_id (source_dest_ranks[i].first);
        std::vector<int> d;
        d.clear();

        for (int j = 0; j < source_dest_ranks[i].second.size(); j++)
        {
            int dest_id = optiq_topology_get_node_id (source_dest_ranks[i].second[j]);
            d.push_back(dest_id);
        }

        std::pair<int, std::vector<int> > p = make_pair (source_id, d);
        source_dest_ids.push_back(p);
    }
}

void optiq_schedule_map_from_pathids_to_pathranks (std::vector<struct path *> &path_ids, std::vector<std::pair<int, std::vector<int> > > &source_dest_ranks, std::vector<struct path *> &path_ranks)
{
    for (int i = 0; i < source_dest_ranks.size(); i++)
    {
        int source_rank = source_dest_ranks[i].first;
        int source_id = optiq_topology_get_node_id(source_rank);

        for (int j = 0; j < source_dest_ranks[i].second.size(); j++) 
        {
            int dest_rank = source_dest_ranks[i].second[j];
            int dest_id = optiq_topology_get_node_id (dest_rank);

            /* Search for path with the same source id and dest id */
            for (int k = 0; k < path_ids.size(); k++)
            {
                if (source_id == path_ids[k]->arcs.front().u && dest_id == path_ids[k]->arcs.back().v)
                {
                    /*Map the path_ids to path_ranks */
                    struct path *path_rank = (struct path *) calloc(1, sizeof(struct path));
                    *path_rank = *path_ids[k];

                    /*Map randomly node id to rank*/
                    for (int h = 0; h < path_rank->arcs.size(); h++)
                    {
                        if (topo->num_ranks_per_node > 1) 
                        {
                            path_rank->arcs[h].u = path_rank->arcs[h].u * topo->num_ranks_per_node + source_rank % topo->num_ranks_per_node;
                            path_rank->arcs[h].v = path_rank->arcs[h].v * topo->num_ranks_per_node + source_rank % topo->num_ranks_per_node;
                        }
                        else 
                        {
                            path_rank->arcs[h].u = path_rank->arcs[h].u;
                            path_rank->arcs[h].v = path_rank->arcs[h].v;
                        }
                    }

                    /*Map exact source rank and dest rank*/
                    path_rank->arcs.front().u = source_rank;
                    path_rank->arcs.back().v = dest_rank;

                    path_rank->source_id = source_id;
                    path_rank->source_rank = source_rank;
                    path_rank->dest_id = dest_id;
                    path_rank->dest_rank = dest_rank;

                    path_ranks.push_back(path_rank);
                }
            }
        }
    }

    /* Assign path id */
    for (int i = 0; i < path_ranks.size(); i++) {
        path_ranks[i]->path_id = i;
    }
}

void optiq_schedule_print_sourcedests(std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    for (int i = 0; i < source_dests.size(); i++)
    {
        for (int j = 0; j < source_dests[i].second.size(); j++)
        {
            printf("source %d dest %d\n", source_dests[i].first, source_dests[i].second[j]);
        }
    }
}
