#include "optiq.h"
#include "mpi_benchmark.h"

#include <mpi.h>

void gen_dests(std::vector<int> &dests)
{
    struct topology *topo = optiq_topology_get();

    int coord[5] = {0, 0, 0, 0, 0};

    for (int i = 4; i >= 0; i--)
    {
	for (int j = 1; j < topo->size[i]; j++) {
	    coord[i] = j;
	    dests.push_back(optiq_topology_compute_node_id(topo->num_dims, topo->size, coord));
	}
    }
}

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

    int datalen = atoi(argv[1]) * 1024;
    char *sendbuf = (char *) malloc (datalen);
    char *recvbuf = (char *) calloc (1, datalen);

    std::vector<int> dests;
    dests.push_back(1);
    //dests.push_back(15);
    //dests.push_back(31);
    //dests.push_back(255);
    //dests.push_back(191);
    //dests.push_back(767);
    //gen_dests(dests);

    if (world_rank == 0) {
	printf("Start to benchmark optiq_alltoallv\n");

	for (int i = 0; i < dests.size(); i++) {
	    printf("Dests = %d ", dests[i]);
	}
	printf("\n");
    }

    int recvrank = 0, sendrank = 0;

    for (int nbytes = 1024; nbytes <= datalen; nbytes *= 2)
    {
	for (int i = 0; i < dests.size(); i++) 
	{
	    memset(sendcounts, 0, world_size * sizeof(int));
	    memset(recvcounts, 0, world_size * sizeof(int));

	    MPI_Barrier(MPI_COMM_WORLD);

	    recvrank = dests[i];

	    if (world_rank == sendrank) {
		sendcounts[recvrank] = nbytes;
	    }

	    if (world_rank == recvrank) {
		recvcounts[sendrank] = nbytes;
	    }

	    //optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	    for (int chunk = nbytes; chunk <= nbytes; chunk *= 2)
	    {
		schedule->chunk_size = chunk;

		optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

		if (world_rank == 0) {
		    printf("\n");
		    optiq_path_print_paths(schedule->paths);
		    printf("hops = %d dest = %d chunk_size = %d message_size = %d", schedule->paths[0]->arcs.size(), recvrank, schedule->chunk_size, nbytes);
		}

		/*printf("Rank %d come to here\n", world_rank);*/

		opi.iters = 1;
		optiq_opi_collect(world_rank);
		optiq_opi_clear();
	    }
	}
    }

    if (world_rank == 0) {
	printf("Finished benchmarking optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
