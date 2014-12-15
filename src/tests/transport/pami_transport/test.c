#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "pami_transport.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int source = 0, dest = 1;

    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)malloc(sizeof(struct optiq_pami_transport));

    struct optiq_rput_cookie rput_cookie;
    rput_cookie.val = 1;
    rput_cookie.mr_val = 1;
    rput_cookie.dest = dest;
    rput_cookie.pami_transport = pami_transport;
    pami_transport->extra.rput_cookie = &rput_cookie;

    int buf_size = 4 * 1024 * 1024;
    void *remote_buf = malloc(buf_size);
    void *local_buf = malloc(buf_size);

    optiq_memregion local_mr, remote_mr;
    pami_transport->extra.remote_mr = &remote_mr;
    pami_transport->extra.remote_mr->offset = 0;
    pami_transport->extra.local_mr = &local_mr;
    pami_transport->extra.local_mr->offset = 0;

    size_t bytes;
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, remote_buf, buf_size, &bytes, &remote_mr.mr);

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

    optiq_pami_init(pami_transport);

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    if (world_rank == source) {
	int nbytes = 128 * 1024;
 
	for (int sent_bytes = 0; sent_bytes < buf_size; sent_bytes += nbytes) {
	    rput_cookie.mr_val = 1;
	    rput_cookie.val = 1;

	    /*Notify the size, ask for mem region*/
	    optiq_pami_send_immediate(pami_transport->context, MR_REQUEST, NULL, 0, &nbytes, sizeof(int), pami_transport->endpoints[dest]);
	    while(rput_cookie.mr_val > 0) {
		PAMI_Context_advance(pami_transport->context, 100);
	    }

	    /*Actual rput data*/ 
	    optiq_pami_rput(pami_transport->client, pami_transport->context, &local_mr.mr, local_mr.offset, nbytes, pami_transport->endpoints[dest], &pami_transport->extra.remote_mr->mr, pami_transport->extra.remote_mr->offset, &rput_cookie);

	    /*Notify that rput is done*/
	    while(rput_cookie.val > 0) {
		PAMI_Context_advance(pami_transport->context, 100);
	    }
	    optiq_pami_send_immediate(pami_transport->context, RPUT_DONE, NULL, 0, NULL, 0, pami_transport->endpoints[rput_cookie.dest]);
	}
    }

    if (world_rank == dest) {
	int nbytes = 128 * 1024;
	rput_cookie.val = buf_size/nbytes;
        while(rput_cookie.val > 0) {
            PAMI_Context_advance(pami_transport->context, 100);
        }
    }

    uint64_t end = GetTimeBase();

    double t = (double)(end-start)/1.6e3;
    double bw = buf_size/t/1024/1024*1e6;

    printf("Rank %d done test t = %8.4f (microsecond), bw = %8.4f (MB/s)\n", world_rank, t, bw);

    return 0;
}
