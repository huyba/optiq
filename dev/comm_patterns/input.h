#ifndef OPTIQ_INPUT
#define OPTIQ_INPUT

#include "job.h"
#include "path.h"

int optiq_input_convert_sendcounts_to_jobs (int *sendcounts, std::vector<struct job> *jobs, int world_size, int ranks_per_node);

void optiq_input_convert_jobs_to_alltoallv (std::vector<struct job> &jobs, char **sendbuf, int **sendcounts, int **sdispls, char **recvbuf, int **recvcounts, int **rdispls, int size, int rank);

#endif
