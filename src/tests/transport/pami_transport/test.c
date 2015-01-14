#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "multibfs.h"
#include "topology.h"
#include "pami_transport.h"

using namespace std;

void flow_create(int world_rank, int *next_dest)
{
    if (world_rank == 0) {
	next_dest[0] = 1;
	next_dest[1] = 2;
    }
    if (world_rank == 1) {
	next_dest[0] = 3;
    }
    if (world_rank == 3) {
	next_dest[0] = 5;
	next_dest[1] = 1;
	next_dest[3] = 1;
    }
    if (world_rank == 5) {
	next_dest[0] = 7;
	next_dest[1] = 3;
	next_dest[3] = 3;
    }
    if (world_rank == 7) {
	next_dest[0] = 6;
	next_dest[1] = 5;
	next_dest[3] = 5;
    }
    if (world_rank == 6) {
	next_dest[0] = 4;
	next_dest[1] = 7;
	next_dest[2] = 4;
	next_dest[3] = 7;
    }
    if (world_rank == 4) {
	next_dest[0] = 2;
	next_dest[1] = 6;
	next_dest[2] = 2;
    }
    if (world_rank == 2) {
	next_dest[1] = 4;
    }
}

void build_next_dest(int world_rank, int *next_dest, std::vector<struct path> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
	for (int j = 0; j < complete_paths[i].arcs.size(); j++)
	{
	    if (complete_paths[i].arcs[j].v == world_rank) 
	    {
		next_dest[i] = complete_paths[i].arcs[j].u;
	    }
	}
    }
}


int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_size < 8) {
	printf("Need at least 8 nodes\n");
	exit(0);
    }

    /*Start the configuration for the test*/
    int num_dims = 5;
    int size[5];
    optiq_topology_get_size_bgq(size);

    int nbytes = 32 * 1024;
    int send_buf_size = 1 * 1024 * 1024;
    int recv_buf_size = world_size * 1024 * 1024;
    int forward_buf_size = OPTIQ_FORWARD_BUFFER_SIZE;

    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    int num_jobs = 4;
    int expecting_length = world_size * 1024 * 1024;

    int *rdispls = (int *) malloc (sizeof(int) * world_size);
    for (int i = 0; i < world_size; i++) {
	rdispls[i] = i * send_buf_size;
    }

    int num_rput_cookies = 32 * 1024;
    int num_message_headers = 32 * 1024;

    std::vector<struct path> complete_paths;
    build_paths(complete_paths, num_dims, size, num_dests, dests);

    /*if (world_rank == 0) {
	optiq_path_print_paths(complete_paths);
    }*/

    MPI_Barrier(MPI_COMM_WORLD);

    int *next_dest = (int*)malloc(sizeof(int) * complete_paths.size());

    build_next_dest(world_rank, next_dest, complete_paths);

    int *flow_id = (int *)malloc(sizeof(int) * num_dests);
    int *final_dest = (int *)malloc(sizeof(int) * num_dests);

    bool isSource = false, isDest = false;

    int index = 0;
    for (int i = 0; i < complete_paths.size(); i++) {
	if (complete_paths[i].arcs.front().u == world_rank) {
	    isDest = true;
	}

	if (complete_paths[i].arcs.back().v == world_rank) {
	    isSource = true;
	    flow_id[index] = i;
	    final_dest[index] = complete_paths[i].arcs.front().u;
	    index++;
	}
    }

    /*if (isDest) {
	printf("Rank %d is dest\n", world_rank);
    }

    if (isSource) {
	printf("Rank %d is source\n", world_rank);
	for (int i = 0; i < num_dests; i++) {
	    printf("Rank %d is source flow_id = %d, dest = %d\n", world_rank, flow_id[i], final_dest[i]);
	}
    }*/

    /*End the configuration for the test*/

    MPI_Barrier(MPI_COMM_WORLD);

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    for (int i = 0; i < num_rput_cookies; i++) {
	struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)calloc(1, sizeof(struct optiq_rput_cookie));
	rput_cookie->pami_transport = pami_transport;
	pami_transport->extra.rput_cookies.push_back(rput_cookie);
    }

    for (int i = 0; i < num_message_headers; i++) {
	struct optiq_message_header *message_header = (struct optiq_message_header *)calloc(1, sizeof(struct optiq_message_header));
	pami_transport->extra.message_headers.push_back(message_header);
    }

    char *recv_buf = (char *) malloc(recv_buf_size);
    char *forward_buf = (char *) malloc(forward_buf_size);
    char *send_buf = (char *) malloc(send_buf_size);

    for (int i = 0; i < send_buf_size; i++) {
	send_buf[i] = i % 128;
    }

    pami_transport->extra.remaining_jobs = num_jobs;
    pami_transport->extra.next_dest = next_dest;
    pami_transport->extra.expecting_length = expecting_length;
    pami_transport->extra.sent_bytes = 0;

    struct optiq_memregion send_mr, forward_mr, recv_mr;

    pami_transport->extra.forward_mr = &forward_mr;
    pami_transport->extra.forward_mr->offset = 0;

    pami_transport->extra.send_mr = &send_mr;
    pami_transport->extra.send_mr->offset = 0;

    pami_transport->extra.recv_mr = &recv_mr;
    pami_transport->extra.recv_mr->offset = 0;

    pami_transport->extra.rdispls = rdispls;

    pami_transport->extra.recv_bytes = (int *) malloc (sizeof(int) * world_size);
    for (int i = 0; i < world_size; i++) {
        pami_transport->extra.recv_bytes[i] = 0;
    }

    //pami_transport->extra.far_mr = &far_mr;
    
    optiq_pami_init(pami_transport);

    size_t bytes;
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, forward_buf, forward_buf_size, &bytes, &forward_mr.mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < forward_buf_size) {
	printf("Registered less\n");
    }

    result = PAMI_Memregion_create (pami_transport->context, send_buf, send_buf_size, &bytes, &send_mr.mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < send_buf_size) {
	printf("Registered less\n");
    }

    result = PAMI_Memregion_create (pami_transport->context, recv_buf, recv_buf_size, &bytes, &recv_mr.mr);

    if (result != PAMI_SUCCESS) {
        printf("No success\n");
    } else if (bytes < recv_buf_size) {
        printf("Registered less\n");
    }

    if (world_rank == 0) {
	printf("num_jobs = %d, expecting_length = %d\n", pami_transport->extra.remaining_jobs, pami_transport->extra.expecting_length);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    if (isSource) {
	for (int offset = 0; offset < send_buf_size; offset += nbytes) {
	    for (int i = 0; i < num_dests; i++) {
		struct optiq_message_header *header = pami_transport->extra.message_headers.back();
		pami_transport->extra.message_headers.pop_back();

		header->length = nbytes;
		header->source = world_rank;
		header->dest = final_dest[i];
		header->flow_id = flow_id[i];

		memcpy(&header->mem, &send_mr, sizeof(struct optiq_memregion));
		header->mem.offset = offset;
		header->original_offset = offset;

		pami_transport->extra.send_headers.push_back(header);
	    }
	}
    }

    while (pami_transport->extra.remaining_jobs > 0) 
    {
	PAMI_Context_advance(pami_transport->context, 100);

	/*If all jobs are done*/
	if (pami_transport->extra.remaining_jobs == 0) {
            break;
        }

	/*If a destination has received all of its data*/
	if (isDest && pami_transport->extra.expecting_length == 0) {
            for (int i = 0; i < world_size; i++) {
                optiq_pami_send_immediate(pami_transport->context, JOB_DONE, NULL, 0, NULL, 0, pami_transport->endpoints[i]);
            }
            pami_transport->extra.expecting_length = -1;
	    //printf("Finishing\n");
        }

	/*if (isDest && pami_transport->extra.expecting_length < -512*1024) {
	    for (int i = 0; i < world_size; i++) {
		printf("Rank %d received %d bytes from %d\n", world_rank, pami_transport->extra.recv_bytes[i], i);
	    }
	    printf("Oooop, it is less than 0 %d\n", pami_transport->extra.expecting_length);
	}
	
	if (isSource && //pami_transport->extra.sent_bytes == buf_size && 
		pami_transport->extra.send_headers.size() + pami_transport->extra.forward_headers.size() == 0) {
	    printf("Rank %d done sending/forwarding, sent_bytes = %d\n", world_rank, pami_transport->extra.sent_bytes);
	    //isSource = false;
	}*/

	/*If there is a request to send a message*/
	if (pami_transport->extra.send_headers.size() + pami_transport->extra.forward_headers.size() > 0)
	{
	    struct optiq_message_header *header = NULL;
	    if (pami_transport->extra.send_headers.size() > 0) 
	    {
		header = pami_transport->extra.send_headers.front();
		pami_transport->extra.send_headers.erase(pami_transport->extra.send_headers.begin());
	    }
	    else if (pami_transport->extra.forward_headers.size() > 0) 
	    {
		header = pami_transport->extra.forward_headers.front();
		pami_transport->extra.forward_headers.erase(pami_transport->extra.forward_headers.begin());
	    }

	    header->header_id = pami_transport->extra.global_header_id;
	    pami_transport->extra.global_header_id++;
	    pami_transport->extra.processing_headers.push_back(header);

	    /*Notify the size, ask for mem region*/
            int dest = next_dest[header->flow_id];

	    /*If the next destination is final destination*/
	    if (dest == header->dest) {
		optiq_pami_send_immediate(pami_transport->context, MR_DESTINATION_REQUEST, &header->header_id, sizeof(int), &header->source, sizeof(int), pami_transport->endpoints[dest]);
	    } else {
		optiq_pami_send_immediate(pami_transport->context, MR_FORWARD_REQUEST, &header->header_id, sizeof(int), &header->length, sizeof(int), pami_transport->endpoints[dest]);
	    }
	}
	
	/*If there is a mem region ready to be transferred*/
	if (pami_transport->extra.mr_responses.size() > 0) 
	{
	    struct optiq_memregion far_mr = pami_transport->extra.mr_responses.front();
	    pami_transport->extra.mr_responses.erase(pami_transport->extra.mr_responses.begin());

	    /*Search for the message with the same header_id*/
	    struct optiq_message_header *header = NULL;
	    for (int i = 0; i < pami_transport->extra.processing_headers.size(); i++) 
	    {
		if (pami_transport->extra.processing_headers[i]->header_id == far_mr.header_id) 
		{
		    header = pami_transport->extra.processing_headers[i];
		    pami_transport->extra.processing_headers.erase(pami_transport->extra.processing_headers.begin() + i);
		    break;
		}
	    }
	    
	    /*Actual rput data*/
	    struct optiq_rput_cookie *rput_cookie = pami_transport->extra.rput_cookies.back();
            pami_transport->extra.rput_cookies.pop_back();

	    int dest = pami_transport->extra.next_dest[header->flow_id];
	    rput_cookie->message_header = header;
	    rput_cookie->dest = dest;


	    /*Rput for destination message. This is because if the next dest is final dest, the dest only gives the offset of the source, not offset for each chunk*/
	    if (dest == header->dest) {
		far_mr.offset += header->original_offset;
	    }

	    optiq_pami_rput(pami_transport->client, pami_transport->context, &header->mem.mr, header->mem.offset, header->length, pami_transport->endpoints[dest], &far_mr.mr, far_mr.offset, rput_cookie);

	    /*Now the header will contain the far memregion instead of local memregion*/
	    memcpy(&header->mem, &far_mr, sizeof(struct optiq_memregion));
	}

	/*If a put is done, notify the remote destination*/
	if (pami_transport->extra.complete_rputs.size() > 0) 
	{
	    struct optiq_rput_cookie *complete_rput = pami_transport->extra.complete_rputs.front();
	    pami_transport->extra.complete_rputs.erase(pami_transport->extra.complete_rputs.begin());

	    struct optiq_message_header *complete_header = complete_rput->message_header;

	    /*Notify that rput is done*/
	    optiq_pami_send_immediate(pami_transport->context, RPUT_DONE, NULL, 0, complete_header, sizeof(struct optiq_message_header), pami_transport->endpoints[complete_rput->dest]);

	    pami_transport->extra.message_headers.push_back(complete_header);
	    pami_transport->extra.rput_cookies.push_back(complete_rput);
	}
    }

    uint64_t end = GetTimeBase();

    double max_t, t = (double)(end-start)/1.6e3;

    MPI_Reduce(&t, &max_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    int max_buffer_size;
    MPI_Reduce(&pami_transport->extra.forward_mr->offset, &max_buffer_size, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
	double bw = num_dests * expecting_length/max_t/1024/1024*1e6;
	printf("Done test t = %8.4f (microsecond), bw = %8.4f (MB/s)\n", t, bw);
	printf("Max buffer size = %d\n", max_buffer_size);
    } 

    if (isDest) {
	char *test_buf = (char *) malloc (recv_buf_size);
	for (int i = 0; i < recv_buf_size; i++) {
	    test_buf[i] = i%128;
	}
	if (memcmp(test_buf, recv_buf, recv_buf_size) != 0) {
	    printf("Rank %d Received invalid data\n", world_rank);
	}
    }

    MPI_Finalize();

    return 0;
}
