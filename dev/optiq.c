#include "optiq.h"

void optiq_init()
{
    optiq_topology_init();
    optiq_pami_transport_init();
    optiq_multibfs_init();
    optiq_schedule_init();
}

void optiq_finalize()
{
    optiq_topology_finalize();
    optiq_pami_transport_finalize();
    optiq_multibfs_finalize();
    optiq_schedule_finalize();
}

int optiq_schedule_get_pair(int *sendcounts, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;
    int num_dests = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
	    num_dests++;
	}
    }

    int *dests = (int *) malloc (sizeof(int) * num_dests);
    int j = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
            dests[j] = i;
	    j++;
        }
    }

    int *all_num_dests = pami_transport->transport_info.all_num_dests;

    for (int i = 0; i < world_size; i++) {
	optiq_pami_send_immediate(pami_transport->context, BROADCAST_NUM_DESTS, 0, 0, &num_dests, sizeof(int), pami_transport->endpoints[i]);
    }

    int **all_dests = (int **) malloc (sizeof(int *) * world_size);
    for (int i = 0; i < world_size; i++) {
	all_dests[i] = (int *) malloc (sizeof (int) * all_num_dests[i]);
    }

    for (int i = 0; i < world_size; i++) {
	optiq_pami_transport_rput(dests, num_dests * sizeof(int), world_rank, all_dests[world_rank], i);
    }

    for (int i = 0; i < world_size; i++) 
    {
	std::vector<int> d;
	d.clear();

	for (int j = 0; j < all_num_dests[i]; j++) {
	    d.push_back(all_dests[i][j]);
	}

	if (d.size() > 0) {
	    std::pair<int, std::vector<int> > p = make_pair(i, d);
	    source_dests.push_back(p);
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
    /* Gather all pairs of source-dest and demand */
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int num_jobs = optiq_schedule_get_pair (sendcounts, source_dests);

    /* Search for paths */
    std::vector<struct path *> paths;
    optiq_algorithm_search_path (paths, source_dests, bfs);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

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
    struct optiq_schedule *schedule = optiq_schedule_get();
    schedule->rdispls = rdispls;
    schedule->recv_len = recv_len;

    /* Register memories */


    /* Add local jobs */
    schedule->local_jobs.clear();
    for (int i = 0; i < paths.size(); i++)
    {
        if (paths[i]->arcs.front().u == pami_transport->rank) 
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

                new_job.source_rank = pami_transport->rank;
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
    int chunk_size = sendcounts[0]/2;
    optiq_schedule_split_jobs_multipaths (pami_transport, schedule->local_jobs, chunk_size);

    /*Reset a few parameters*/
    optiq_schedule_set (*schedule, num_jobs, pami_transport->size);
}

/* Destroy the registered memory regions */
void optiq_schedule_destroy()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();
    
    optiq_schedule_mem_destroy(*schedule, pami_transport);
}

void optiq_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    optiq_schedule_build (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    optiq_pami_transport_execute (pami_transport);

    optiq_schedule_destroy ();
}
