#include <stdlib.h>
#include "comm_mem.h"


/* Allocate mem for send/recv, also assign counts, displacements*/
int optiq_comm_mem_allocate (std::vector<std::pair<int, std::vector<int> > > &source_dests, int count, struct optiq_comm_mem &comm_mem, int rank, int world_size)
{
    comm_mem.sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    comm_mem.sdispls = (int *)calloc(1, sizeof(int) * world_size);

    comm_mem.rdispls = (int *) malloc(sizeof(int) * world_size);
    comm_mem.recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    int send_len = 0;
    int recv_len = 0;

    for (int i = 0; i < source_dests.size(); i++)
    {
	if (source_dests[i].first == rank)
	{
	    send_len = count * source_dests[i].second.size();
	    comm_mem.send_buf = (char *) malloc (send_len);

	    for (int j = 0; j < source_dests[i].second.size(); j++)
	    {
		int dest_id = source_dests[i].second[j];
		comm_mem.sendcounts[dest_id] = count;
		comm_mem.sdispls[dest_id] = j * count;
	    }
	}
    }

    comm_mem.send_len = send_len;

    int num_sources = 0;

    for (int i = 0; i < source_dests.size(); i++)
    {
	for (int j = 0; j < source_dests[i].second.size(); j++)
	{
	    if (source_dests[i].second[j] == rank)
	    {
		int source_id = source_dests[i].first;
		comm_mem.recvcounts[source_id] = count;
		comm_mem.rdispls[source_id] = num_sources * count;
		num_sources++;
	    }
	}
    }

    if (num_sources > 0) {
	recv_len = num_sources * count;
	comm_mem.recv_buf = (char *) malloc (recv_len);
    }
    comm_mem.recv_len = recv_len;

    return num_sources;
}

void optiq_comm_mem_delete(struct optiq_comm_mem &comm_mem)
{
    free(comm_mem.sendcounts);
    free(comm_mem.sdispls);
    if (comm_mem.send_len > 0) {
	free(comm_mem.send_buf);
    }

    free(comm_mem.recvcounts);
    free(comm_mem.rdispls);
    if (comm_mem.recv_len > 0) {
	free(comm_mem.recv_buf);
    }
}
