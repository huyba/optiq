#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "pami_transport.h"

using namespace std;

void flow_create(int world_rank, int *next_dest)
{
    if (world_rank == 0) {
	next_dest[0] = 1;
    }
    if (world_rank == 1) {
	next_dest[0] = 3;
    }
    if (world_rank == 3) {
	next_dest[0] = 5;
    }
    if (world_rank == 5) {
	next_dest[0] = 7;
    }
    if (world_rank == 7) {
	next_dest[0] = 6;
    }
    if (world_rank == 6) {
	next_dest[0] = 4;
    }
    if (world_rank == 4) {
	next_dest[0] = 2;
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

    int source = 0, dest = 0;
    bool isSource = false, isDest = false;
    int next_dest[1];

    flow_create(world_rank, next_dest);

    if (world_rank == 0) {
	isSource = true;
    }
    if (world_rank  == 2) {
	isDest = true;
    }

    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    pami_transport->extra.remaining_jobs = 1;
    pami_transport->extra.next_dest = next_dest;

    struct optiq_rput_cookie *rput_cookie;

    int buf_size = 1 * 1024 * 1024;
    char *remote_buf = (char *) malloc(buf_size);
    char *local_buf = (char *) malloc(buf_size);

    for (int i = 0; i < buf_size; i++) {
	local_buf[i] = i % 128;
    }

    pami_transport->extra.expecting_length = buf_size;

    struct optiq_memregion local_mr, near_mr, far_mr;

    size_t bytes;
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, remote_buf, buf_size, &bytes, &near_mr.mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < buf_size) {
	printf("Registered less\n");
    }

    result = PAMI_Memregion_create (pami_transport->context, local_buf, buf_size, &bytes, &local_mr.mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < buf_size) {
	printf("Registered less\n");
    }

    pami_transport->extra.near_mr = &near_mr;
    pami_transport->extra.near_mr->offset = 0;
    pami_transport->extra.far_mr = &far_mr;
    pami_transport->extra.far_mr->offset = 0;

    int num_headers = 64;
    for (int i = 0; i < num_headers; i++) {
	struct optiq_message_header *header = (struct optiq_message_header *)malloc(sizeof(struct optiq_message_header));

	pami_transport->extra.message_headers.push_back(header);
    }

    int num_rput_cookies = 64;
    for (int i = 0; i < num_rput_cookies; i++) {
	struct optiq_rput_cookie *cookie = (struct optiq_rput_cookie *)malloc(sizeof(struct optiq_rput_cookie));
	cookie->pami_transport = pami_transport;

	pami_transport->extra.rput_cookies.push_back(cookie);
    }

    optiq_pami_init(pami_transport);

    int nbytes = 32 * 1024;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    struct optiq_message_header *header = NULL;

    if (isSource) {
	int final_dest = 2;

	for (int offset = 0; offset < buf_size; offset += nbytes) {
	    header = pami_transport->extra.message_headers.back();
	    pami_transport->extra.message_headers.pop_back();

	    header->source = world_rank;
	    header->dest = final_dest;
	    header->length = nbytes;
	    header->flow_id = 0;
	    header->mem = local_mr;
	    header->mem.offset = offset;

	    pami_transport->extra.local_messages.push_back(header);	    
	}
    }

    printf("Rank %d comes to here\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    
    while (pami_transport->extra.remaining_jobs > 0) {

	/*If no message to transfer*/
	if (pami_transport->extra.local_messages.size() == 0 && pami_transport->extra.forward_messages.size() == 0) {
	    PAMI_Context_advance(pami_transport->context, 100);

	    if (pami_transport->extra.remaining_jobs == 0) {
		break;
	    }
	}

	/*Get message header from one of the vectors*/
	if (pami_transport->extra.local_messages.size() > 0) 
	{
	    header = pami_transport->extra.local_messages.front();
	    pami_transport->extra.local_messages.erase(pami_transport->extra.local_messages.begin());
	}
	else if (pami_transport->extra.forward_messages.size() > 0) 
	{
	    header = pami_transport->extra.forward_messages.front();
	    pami_transport->extra.forward_messages.erase(pami_transport->extra.forward_messages.begin());
	}

	/*Notify the size, ask for mem region*/
	dest = next_dest[header->flow_id];
	pami_transport->extra.mr_val = 1;
	optiq_pami_send_immediate(pami_transport->context, MR_REQUEST, NULL, 0, &header->length, sizeof(int), pami_transport->endpoints[dest]);

	while(pami_transport->extra.mr_val > 0) 
	{
	    PAMI_Context_advance(pami_transport->context, 100);
	}

	printf("Rank %d before rput\n", world_rank);

	/*Actual rput data*/
	rput_cookie = pami_transport->extra.rput_cookies.back();
	pami_transport->extra.rput_cookies.pop_back();

	rput_cookie->header = header;
	optiq_pami_rput(pami_transport->client, pami_transport->context, &header->mem.mr, header->mem.offset, header->length, pami_transport->endpoints[dest], &pami_transport->extra.far_mr->mr, pami_transport->extra.far_mr->offset, rput_cookie);
    }

    uint64_t end = GetTimeBase();

    double t = (double)(end-start)/1.6e3;
    double bw = buf_size/t/1024/1024*1e6;

    if (isSource || isDest) {
	printf("Rank %d done test t = %8.4f (microsecond), bw = %8.4f (MB/s)\n", world_rank, t, bw);
    } else {
	//printf("Rank %d not involved into the test\n", world_rank);
    }

    if (isDest) {
	if (memcmp(local_buf, remote_buf, buf_size) != 0) {
	    printf("Rank %d Received invalid data\n", world_rank);
	}
    }

    MPI_Finalize();

    return 0;
}
