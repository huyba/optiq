#include <stdlib.h>
#include <stdio.h>

#include <vector>

#include <mpi.h>

#include "input.h"

int optiq_input_convert_sendcounts_to_jobs (int *sendcounts, std::vector<struct job> *jobs, int world_size, int ranks_per_node)
{
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

    int *all_num_dests = (int *) calloc (1, sizeof(int) * world_size);

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
	    for (int j = offset; j < offset + all_num_dests[i]; j++) 
	    {
		distinguished_dests[all_dests[j]] = true;

		if (jobs != NULL)
		{
		    struct job newjob;
		    newjob.source_rank = i;
		    newjob.source_id = newjob.source_rank / ranks_per_node;
		    newjob.dest_rank = all_dests[j];
		    newjob.dest_id = newjob.dest_rank / ranks_per_node;
		    newjob.job_id = id;

		    jobs->push_back(newjob);

		    id++;
		}
	    }

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
