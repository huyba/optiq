#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include "input.h"

int optiq_input_get_pair(int *sendcounts, std::vector<std::pair<int, std::vector<int> > > &source_dests, std::vector<struct job> *jobs)
{
    source_dests.clear();

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;
    int num_dests = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
	    num_dests++;
	}
    }

    int *dests = NULL;
    if (num_dests > 0) {
	dests = (int *) malloc (sizeof(int) * num_dests);
    }
    int j = 0;

    for (int i = 0; i < world_size; i++) {
	if (sendcounts[i] != 0) {
	    dests[j] = i;
	    j++;
	}
    }

    int *all_num_dests = pami_transport->sched->all_num_dests;

    MPI_Allgather(&num_dests, 1, MPI_INT, all_num_dests, 1, MPI_INT, MPI_COMM_WORLD);

    int offset = 0;
    int *displs = (int *)malloc (sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) 
    {
	displs[i] = offset;
	offset += all_num_dests[i];
    }

    int *all_dests = (int *) malloc (sizeof(int) * offset);

    MPI_Allgatherv(dests, num_dests, MPI_INT, all_dests, all_num_dests, displs, MPI_INT, MPI_COMM_WORLD);

    bool *distinguished_dests = (bool *) calloc (1, sizeof(bool) * world_size);
    offset = 0;

    int id = 0;
    for (int i = 0; i < world_size; i++)
    {
	if (all_num_dests[i] > 0) 
	{
	    std::vector<int> d;

	    for (int j = offset; j < offset + all_num_dests[i]; j++) 
	    {
		d.push_back (all_dests[j]);
		distinguished_dests[all_dests[j]] = true;

		if (jobs != NULL)
		{
		    struct job newjob;
		    newjob.source_rank = i;
		    newjob.dest_rank = all_dests[j];
		    newjob.job_id = id;

		    jobs->push_back(newjob);

		    id++;
		}
	    }

	    std::pair<int, std::vector<int> > p = make_pair(i, d);
	    source_dests.push_back(p);
	}
	offset += all_num_dests[i];
    }

    int num_distinguished_dests = 0;

    for (int i = 0; i < world_size; i++) 
    {
	if (distinguished_dests[i]) {
	    num_distinguished_dests++;
	}
    }

    free (distinguished_dests);
    free (all_dests);

    if (dests != NULL) {
	free(dests);
    }

    free(displs);

    return num_distinguished_dests;
}

void optiq_input_alltoallv (std::vector<struct job> &jobs)
{
    struct topology *topo = optiq_topology_get();

    if (topo->num_ranks_per_node > 1) 
    {
        for (int i = 0; i < jobs.size(); i++)
        {
            jobs[i].source_id = jobs[i].source_rank / topo->num_ranks_per_node;
            jobs[i].dest_id = jobs[i].dest_rank / topo->num_ranks_per_node;
        }
    }
}
