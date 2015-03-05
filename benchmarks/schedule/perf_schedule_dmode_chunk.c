#include "optiq.h"
#include "mpi_benchmark.h"

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

    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (world_rank == 0) {
        printf("\nTest Various chunk size\n");
    }

    for (int chunk = 1024; chunk <= send_bytes; chunk *= 2)
    {
	if (world_rank == 0) {
	    printf("\nchunk size = %d\n", chunk);
	}

	schedule->chunk_size = chunk;
	schedule->dmode = DQUEUE_ROUND_ROBIN;

	optiq_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	opi.iters = 1;
	optiq_opi_collect(world_rank);
    }

    if (world_rank == 0) {
        printf("\nDynamic choosing chunk size\n");
    }

    schedule->dmode = DQUEUE_ROUND_ROBIN;
    schedule->chunk_size = optiq_schedule_get_chunk_size (send_bytes, world_rank, send_rank);

    optiq_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    opi.iters = 1;
    optiq_opi_collect(world_rank);

    optiq_finalize();

    return 0;
}
