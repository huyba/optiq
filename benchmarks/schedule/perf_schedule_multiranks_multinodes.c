#include "optiq.h"
#include "mpi_benchmark.h"
#include "mpi.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_topology *topo = optiq_topology_get();
    struct optiq_schedule *schedule = optiq_schedule_get();
    struct optiq_algorithm *algorithm = optiq_algorithm_get();

    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    int *rdispls = (int *) calloc(1, sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int nbytes = 1024 * 1024;
    char *sendbuf = (char *) malloc (nbytes);
    char *recvbuf = (char *) calloc (1, nbytes);

    patterns_disjoint_contigous_multiranks_multinodes (sendcounts, recvcounts, world_rank, world_size, topo->num_ranks_per_node, nbytes);

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_benchmark_mpi_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

    if (world_rank == 0) {
        printf("\nTest multi rank with %d ranks per node\n", topo->num_ranks_per_node);
    }

    algorithm->search_alg = OPTIQ_ALG_HOPS_CONSTRAINT;
    algorithm->num_paths_per_pair = 1;

    schedule->dmode = DQUEUE_FORWARD_MESSAGE_FIRST;

    MPI_Barrier(MPI_COMM_WORLD);

    for (int chunk = 1024; chunk <= nbytes; chunk *= 2)
    {
	schedule->chunk_size = chunk;// optiq_schedule_get_chunk_size (send_bytes, world_rank, send_rank);

	optiq_alltoallv (sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	/*printf("Rank %d chunk size %d finished\n", world_rank, chunk);*/

	opi.iters = 1;
	optiq_opi_collect();
	if (world_rank == 0) {
	    optiq_opi_print();
	}
    }

    if (world_rank >= world_size/2) {
	char *testbuf = (char *) calloc (1, nbytes);
	for (int i = 0; i < nbytes; i++) {
            testbuf[i] = i%128;
        }
	if (memcmp(recvbuf, testbuf, nbytes) != 0) {
	    printf("Rank %d receieved invalid data\n", world_rank);
	}
    }

    if (world_rank == 0) {
        printf("Finished testing schedule dqueue modes\n");
    }

    optiq_finalize();

    return 0;
}
