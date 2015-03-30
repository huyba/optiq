#include "optiq.h"
#include "mpi_benchmark.h"

int main(int argc, char **argv)
{
    if (argc < 3) {
	printf("Need at least 2 params: source dest");
	return 0;
    }

    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;

    struct optiq_algorithm *alg = optiq_algorithm_get();
    alg->search_alg = OPTIQ_ALG_MPIPLUS;

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    int *rdispls = (int *) calloc(1, sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int minbytes = 4 * 1024 * 1024;
    int maxbytes = 4 * 1024 * 1024;
    if (argc > 4) {
	minbytes = atoi (argv[4]) * 1024;
    }
    if (argc > 5) {
	maxbytes = atoi (argv[5]) * 1024;
    }
    int chunksize = 16 * 1024;
    if (argc > 3) {
	chunksize = atoi (argv[3]) * 1024;
    }
    char *sendbuf = (char *) malloc (maxbytes);
    char *recvbuf = (char *) calloc (1, maxbytes);

    int send_rank = atoi(argv[1]);
    int recv_rank = atoi(argv[2]);

    if (world_rank == send_rank) {
	for (int i = 0; i < maxbytes; i++) {
	    sendbuf[i] = i % 128;
	}
    }

    char *testbuf;
    if (recv_rank == world_rank)
    {
	testbuf = (char *) malloc (maxbytes);

	for (int i = 0; i < maxbytes; i++) {
	    testbuf[i] = i % 128;
	}
    }

    if (world_rank == 0) {
	printf("Start to benchmark time to send by hops\n");
    }

    odp.print_path_id = true;

    for (int nbytes = 8 * 1024; nbytes <= maxbytes; nbytes *= 2)
    {
	memset (sendcounts, 0, sizeof(int) * world_size);
	memset (recvcounts, 0, sizeof(int) * world_size);

	if (world_rank == send_rank) {
	    sendcounts[recv_rank] = maxbytes;
	    odp.print_debug_msg = true;
	}

	if (world_rank == recv_rank) {
	    recvcounts[send_rank] = maxbytes;
	}

	optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	schedule->auto_chunksize = false;
	schedule->chunk_size = nbytes;
	//odp.print_rput_msg = true;

	optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	opi.iters = 1;
	optiq_opi_collect();

	if (world_rank == 0) {
	    optiq_opi_print();
	}

	//optiq_opi_timestamp_print(world_rank);

	optiq_opi_clear();

	if (recv_rank == world_rank) {
	    if (memcmp (testbuf, recvbuf, maxbytes) != 0) {
		printf("Error at receiving side\n");
	    }
	}
    }

    if (world_rank == 0) {
	printf("Finished benchmarking optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
