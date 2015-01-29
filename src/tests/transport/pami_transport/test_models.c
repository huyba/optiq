#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>
#include <limits.h>
#include <math.h>

#include "mtonbfs.h"
#include "topology.h"
#include "pami_transport.h"
#include "job.h"

using namespace std;

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)
#define TRANSFER_SIZE (32 * 1024)

struct optiq_request {
    int length;
    char *buf;
    int num_tokens;
    int remaining_tokens;
    int offset;
    int current_offset;
    struct path *path;
    struct job *job;
};

void build_next_dest(int world_rank, int *next_dest, std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
	/*Reset the path_id*/
	complete_paths[i]->path_id = i;

	for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
	{
	    if (complete_paths[i]->arcs[j].u == world_rank) 
	    {
		next_dest[i] = complete_paths[i]->arcs[j].v;
	    }
	}
    }
}

void optiq_build_paths_from_file(void *send_buf, int *sendcounts, int *sdispls, void *recv_buf, int *recvcounts, int *rdispls, struct optiq_bulk *bulk, char *filePath)
{
    //uint64_t t0 = GetTimeBase();

    /*Start the configuration for the test*/
    int num_dims = 5;
    int size[5];
    optiq_topology_get_size_bgq(size);

    int world_rank = bulk->pami_transport->rank;
    int world_size = bulk->pami_transport->size;

    uint64_t t0 = GetTimeBase();

    /*Calculate paths to move data*/
    std::vector<struct path *> complete_paths;
    complete_paths.clear();
    optiq_path_read_from_file(filePath, complete_paths);

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t t1 = GetTimeBase();

    build_next_dest(world_rank, bulk->next_dest, complete_paths);

    uint64_t t2 = GetTimeBase();

    int num_jobs = 4;
    int expecting_length = world_size * 1024 * 1024;

    bool isSource = false, isDest = false;

    int index = 0;
    vector<struct job> local_jobs;

    int demand = 1024 * 1024;

    for (int i = 0; i < complete_paths.size(); i++) 
    {
	if (complete_paths[i]->arcs.back().v == world_rank) 
	{
	    isDest = true;
	}

	if (complete_paths[i]->arcs.front().u == world_rank) 
	{
	    isSource = true;

	    bool existing_job = false;
	    int k = 0;

	    for (int j = 0; j < local_jobs.size(); j++)
	    {
		if (local_jobs[j].job_id == complete_paths[i]->job_id) 
		{
		    k = j;
		    existing_job = true;
		    break;
		}
	    }

	    if (!existing_job)
	    {
		struct job new_job;

		new_job.job_id = complete_paths[i]->job_id;
		new_job.paths.push_back(complete_paths[i]);
		new_job.source_id = complete_paths[i]->arcs.front().u;
		new_job.dest_id = complete_paths[i]->arcs.back().v;
		new_job.demand = demand;

		local_jobs.push_back(new_job);
	    } 
	    else 
	    {
		local_jobs[k].paths.push_back(complete_paths[i]);
	    }
	}
    }

    bulk->remaining_jobs = num_jobs;
    bulk->expecting_length = expecting_length;
    bulk->sent_bytes = 0;

    bulk->send_mr.offset = 0;
    bulk->recv_mr.offset = 0;

    bulk->rdispls = rdispls;

    bulk->isDest = isDest;

    uint64_t t3 = GetTimeBase();

    size_t bytes;
    pami_result_t result;
    int send_buf_size = 1 * 1024 * 1024;
    if (isSource) 
    {
	result = PAMI_Memregion_create (bulk->pami_transport->context, send_buf, send_buf_size, &bytes, &bulk->send_mr.mr);

	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < send_buf_size) {
	    printf("Registered less\n");
	}
    }

    int recv_buf_size = 256 * 1024 * 1024;
    if (isDest) 
    {
	result = PAMI_Memregion_create (bulk->pami_transport->context, recv_buf, recv_buf_size, &bytes, &bulk->recv_mr.mr);

	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < recv_buf_size) {
	    printf("Registered less\n");
	}
    }

    int total_flow = 0;
    int total_bytes = 0;
std:vector<struct optiq_request> requests;

    /*if (isSource)
      {
      int min_flow = INT_MAX;

      for (int i = 0; i < local_jobs.size(); i++) 
      {
      total_bytes += local_jobs[i].demand;

      for (int j = 0; j < local_jobs[i].paths.size(); j++) 
      {
      total_flow += local_jobs[i].paths[j]->flow;

      if (min_flow > local_jobs[i].paths[j]->flow)
      {
      min_flow = local_jobs[i].paths[j]->flow;
      }
      }
      }

      for (int i = 0; i < local_jobs.size(); i++)
      {
      int job_flow = 0;

      for (int j = 0; j < local_jobs[i].paths.size(); j++)
      {
      job_flow += local_jobs[i].paths[j]->flow;
      }

      int offset = 0;
      int path_demand = 0;

      for (int j = 0; j < local_jobs[i].paths.size(); j++)
      {

      path_demand = local_jobs[i].paths[j]->flow * local_jobs[i].demand / job_flow;

      printf("Rank %d path_demand = %d, flow = %d, total_flow = %d, demand = %d\n", world_rank, path_demand, local_jobs[i].paths[j]->flow, job_flow, local_jobs[i].demand);

      struct optiq_request request;
      request.length = path_demand;
      request.offset = offset;
      request.current_offset = offset;
      request.path = local_jobs[i].paths[j];
      request.num_tokens = rint (local_jobs[i].paths[j]->flow / min_flow);
      request.job = &local_jobs[i];

      if (j == local_jobs[i].paths.size() - 1)
      {
      request.length = local_jobs[i].demand - offset;
      }

      requests.push_back(request);

      offset += path_demand;
      }
      }

    //printf("Rank %d has %d requests\n", world_rank, requests.size());

    for (int i = 0; i < requests.size(); i++)
    {
    //printf("Rank %d request %d: offset = %d, length = %d, job_id = %d, path_id = %d, num_token = %d\n", world_rank, i, requests[i].offset, requests[i].length, requests[i].job->job_id, requests[i].path->path_id, requests[i].num_tokens);
    }

    int sent_bytes = 0;

    while (sent_bytes < total_bytes)
    {
    bool out_of_tokens = false;

    for (int i = 0; i < requests.size(); i++)
    {
	requests[i].remaining_tokens = requests[i].num_tokens;
    }

    while (!out_of_tokens)
    {
	out_of_tokens = true;

	for (int i = 0; i < requests.size(); i++)
	{
	    if (requests[i].current_offset - requests[i].offset < requests[i].length)
	    {
		if (requests[i].remaining_tokens > 0) 
		{
		    requests[i].remaining_tokens--;

		    struct optiq_message_header *header = bulk->pami_transport->extra.message_headers.back();
		    bulk->pami_transport->extra.message_headers.pop_back();

		    int nbytes = (requests[i].current_offset - requests[i].offset + TRANSFER_SIZE <= requests[i].length ? TRANSFER_SIZE : requests[i].length - (requests[i].current_offset - requests[i].offset));

		    header->length = nbytes;
		    header->source = requests[i].job->source_id;
		    header->dest = requests[i].job->dest_id;
		    header->path_id = requests[i].path->path_id;

		    memcpy(&header->mem, &bulk->send_mr, sizeof(struct optiq_memregion));
		    header->mem.offset = requests[i].current_offset;
		    header->original_offset = requests[i].current_offset;

		    printf("Rank %d to add message job_id = %d, path_id = %d, source = %d, dest = %d, length = %d, offset = %d\n", world_rank, requests[i].job->job_id, header->path_id, header->source, header->dest, header->length, header->original_offset);

		    bulk->pami_transport->extra.send_headers.push_back(header);

		    sent_bytes += nbytes;
		    requests[i].current_offset += nbytes;
		}

		if (requests[i].remaining_tokens > 0 && requests[i].current_offset - requests[i].offset < requests[i].length)
		{
		    out_of_tokens = false;
		}
	    }
	}
    }
}
}
*/

int pid = 0;
int nbytes = 32 * 1024;
if (isSource)
{
    for (int offset = 0; offset < send_buf_size; offset += nbytes) {
	for (int i = 0; i < local_jobs.size(); i++) {
	    struct optiq_message_header *header = bulk->pami_transport->extra.message_headers.back();
	    bulk->pami_transport->extra.message_headers.pop_back();

	    header->length = nbytes;
	    header->source = local_jobs[i].source_id;
	    header->dest = local_jobs[i].dest_id;

	    if (local_jobs[i].paths.size() > 1) 
	    {
		pid = pid % local_jobs[i].paths.size();
		header->path_id = local_jobs[i].paths[pid]->path_id;
		pid++;
	    } 
	    else 
	    {
		header->path_id = local_jobs[i].paths[0]->path_id;
	    }

	    memcpy(&header->mem, &bulk->send_mr, sizeof(struct optiq_memregion));
	    header->mem.offset = offset;
	    header->original_offset = offset;

	    bulk->pami_transport->extra.send_headers.push_back(header);
	}
    }
}

if (bulk->pami_transport->rank == 0) {
    double t = (double)(t3-t2)/1.6e3;
    printf("compute: flow id, final dest %f\n", t);
    t = (double)(t2-t1)/1.6e3;
    printf("build next dest %f\n", t);
    t = (double)(t1-t0)/1.6e3;
    printf("create paths %f\n", t);
}
}

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    char *filePath = argv[1];

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct mtonbfs *bfs = (struct mtonbfs *) calloc (1, sizeof(struct mtonbfs));
    bfs->num_dims = 5;
    optiq_topology_get_size_bgq(bfs->size);
    bfs->num_nodes = 1;
    for (int i = 0; i < bfs->num_dims; i++) {
	bfs->num_nodes *= bfs->size[i];
    }
    bfs->neighbors = optiq_topology_get_all_nodes_neighbors(bfs->num_dims, bfs->size);

    if (world_rank == 0) {
	printf("num_dims = %d, num_nodes = %d\n", bfs->num_dims, bfs->num_nodes);
    }

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    pami_transport->bulk.pami_transport = pami_transport;
    pami_transport->bulk.bfs = bfs;

    pami_transport->bulk.recv_bytes = (int *) calloc (1, sizeof(int) * world_size);
    pami_transport->bulk.final_dest = (int *) calloc (1, sizeof(int) * world_size);
    pami_transport->bulk.flow_id = (int *) calloc (1, sizeof(int) * world_size);

    pami_transport->bulk.next_dest = (int *) calloc (1, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

    optiq_pami_init_extra(pami_transport);
    optiq_pami_init(pami_transport);

    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    int send_bytes = 1 * 1024 * 1024;
    int send_buf_size = num_dests * send_bytes;
    char *send_buf = (char *) malloc(send_buf_size);
    for (int i = 0; i < send_buf_size; i++) {
	send_buf[i] = i % 128;
    }

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);
    for (int i = 0; i < num_dests; i++) 
    {
	sendcounts[dests[i]] = send_bytes;
	sdispls[dests[i]] = i * send_bytes;
    }

    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int recv_buf_size = 0;
    char *recv_buf = NULL;
    int *rdispls = NULL;

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dests[i]) 
	{
	    recv_buf_size = world_size * send_bytes;
	    recv_buf = (char *) malloc(recv_buf_size);
	    rdispls = (int *) malloc(sizeof(int) * world_size);

	    for (int i = 0; i < world_size; i++) 
	    {
		recvcounts[i] = send_bytes;
		rdispls[i] = i * send_bytes;
	    }
	}
    }

    uint64_t t0 = GetTimeBase();

    optiq_build_paths_from_file(send_buf, sendcounts, sdispls, recv_buf, recvcounts, rdispls, &pami_transport->bulk, filePath);

    uint64_t t1 = GetTimeBase();

    optiq_execute_jobs(pami_transport);

    uint64_t t3 = GetTimeBase();

    double max_t, t = (double)(t3 - t1)/1.6e3;

    MPI_Reduce(&t, &max_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    int max_buffer_size;
    MPI_Reduce(&pami_transport->extra.forward_mr->offset, &max_buffer_size, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
	double bw = num_dests * world_size * send_bytes / max_t / 1024 / 1024 * 1e6;
	printf("Done test t = %8.4f (microsecond), bw = %8.4f (MB/s)\n", t, bw);
	printf("Max buffer size = %d\n", max_buffer_size);
    } 

    for (int i = 0; i < num_dests; i++) {
	if (world_rank == dests[i]) {
	    char *test_buf = (char *) malloc (recv_buf_size);
	    for (int i = 0; i < recv_buf_size; i++) {
		test_buf[i] = i%128;
	    }
	    if (memcmp(test_buf, recv_buf, recv_buf_size) != 0) {
		printf("Rank %d Received invalid data\n", world_rank);
	    }
	}
    }

    MPI_Finalize();

    return 0;
}
