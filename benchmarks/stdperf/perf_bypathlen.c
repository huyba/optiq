#include "optiq.h"
#include "mpi_benchmark.h"

int main(int argc, char **argv)
{

    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;
    struct optiq_schedule *sched = optiq_schedule_get();
    struct topology *topo = optiq_topology_get();

    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    int *rdispls = (int *) calloc(1, sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int maxbytes = 8 * 1024 * 1024;
    if (argc > 3) {
	maxbytes = atoi(argv[3]);
    }
    char *sendbuf = (char *) malloc (maxbytes);
    char *recvbuf = (char *) calloc (1, maxbytes);

    if (world_rank == 0) {
	printf("Start to benchmark time to send by hops\n");
    }

    int sendcoord[5] = {0, 0, 0, 0, 0};
    int recvcoord[5];
    int send_rank = optiq_topology_compute_node_id (topo->num_dims, topo->size, sendcoord);
    int recv_rank;
    std::vector<int> recvranks;

    for (int i = 0; i < 5; i++) 
    {
	for (int j = sendcoord[i]+1; j <= topo->size[i]/2; j++) 
	{
	    recvcoord[i] = j;
	    recv_rank = optiq_topology_compute_node_id (topo->num_dims, topo->size, recvcoord);
	    recvranks.push_back(recv_rank);
	}
    }

    for (int i = 0; i < recvranks.size(); i++)
    {
	recv_rank = recvranks[i];

	odp.print_path_id = true;

	for (int nbytes = 1024; nbytes <= maxbytes; nbytes *= 2)
	{
	    sched->auto_chunksize = false;
	    sched->chunk_size = nbytes;

	    memset (sendcounts, 0, sizeof(int) * world_size);
	    memset (recvcounts, 0, sizeof(int) * world_size);

	    if (world_rank == send_rank) {
		sendcounts[recv_rank] = maxbytes;
	    }

	    if (world_rank == recv_rank) {
		recvcounts[send_rank] = maxbytes;
	    }

	    optiq_benchmark_mpi_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	    optiq_alltoallv(sendbuf, sendcounts, sdispls, recvbuf, recvcounts, rdispls);

	    opi.iters = 1;
	    optiq_opi_collect();
	    if (world_rank == 0) {
		printf("chunksize = %d\n", sched->chunk_size);
		optiq_opi_print();
	    }

	    optiq_opi_clear();
	}
    }

    if (world_rank == 0) {

	printf("Finished benchmarking optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
