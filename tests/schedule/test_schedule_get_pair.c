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

    int send_bytes = 1024;
    void *send_buf = malloc (send_bytes);
    int recv_bytes = 1024;
    void *recv_buf = malloc (recv_bytes);

    int send_rank = 0;
    int recv_rank = 1;

    if (world_rank < world_size/2) {
	recv_rank = world_rank + world_size/2;
	sendcounts[recv_rank] = send_bytes;
    }

    if (world_rank >= world_size/2) {
	send_rank = world_rank - world_size/2;
	recvcounts[send_rank] = recv_bytes;
    }

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    int num_sources = optiq_schedule_get_pair (sendcounts, source_dests);

    for (int i = 0; i < source_dests.size(); i++) {
	for (int j = 0; j < source_dests[i].second.size(); j++) {
	    printf("At rank %d Source %d dest %d\n", world_rank, source_dests[i].first, source_dests[i].second[j]);
	}
    }

    optiq_finalize();

    return 0;
}
