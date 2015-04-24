#include "optiq.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    int *rdispls = (int *) calloc(1, sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int send_bytes = 1024 * 1024;
    char *sendbuf = (char *) malloc (send_bytes);
    int recv_bytes = 1024 * 1024;
    char *recvbuf = (char *) calloc (1, recv_bytes);

    int send_rank = 0;
    int recv_rank = 1;

    if (world_rank < world_size/2) {
	recv_rank = world_rank + world_size/2;
	sendcounts[recv_rank] = send_bytes;

	for (int i = 0; i < send_bytes; i++) {
	    sendbuf[i] = i%128;
	}
    }

    if (world_rank >= world_size/2) {
	send_rank = world_rank - world_size/2;
	recvcounts[send_rank] = recv_bytes;
    }

    if (world_rank == 0) {
	printf("Start to test optiq_alltoallv\n");
    }

    /*odp.print_local_jobs = true;
    odp.print_mem_exchange_status = true;
    odp.print_pami_transport_status = true;
    odp.print_rput_msg = true;*/

    int iters = 20;
    for (int i = 0; i < iters; i++) {
	optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);
    }

    /* Validate the result */
    if (world_rank >= world_size/2) {
	char *testbuf = (char *) malloc(recv_bytes);
	for (int i = 0; i < recv_bytes; i++) {
	    testbuf[i] = i % 128;
	}

	if (memcmp(recvbuf, testbuf, recv_bytes) != 0) {
	    printf("Rank %d received corrupted data\n", world_rank);
	}
    }

    //if (world_rank == 0) {
        printf("Rank %d Finished testing optiq_alltoallv\n", world_rank);
    //}

    opi.iters = iters;
    optiq_opi_collect();
    if (world_rank == 0) {
        optiq_opi_print();
    }

    optiq_finalize();

    return 0;
}
