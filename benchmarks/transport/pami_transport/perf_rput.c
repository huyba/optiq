#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <mpi.h>

#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    optiq_pami_transport_init();

    int local_rank = 0;
    int rput_bytes = 128 * 1024 * 1024;
    void *local_buf = malloc (rput_bytes);

    for (int i = 0; i < rput_bytes; i++) {
	((char*)local_buf)[i] = i % 128;
    }

    int remote_rank = 1;
    void *remote_buf = malloc (rput_bytes);

    int iters = 1;

    for (int nbytes = 1024; nbytes <= rput_bytes; nbytes *= 2)
    {
	MPI_Barrier(MPI_COMM_WORLD);

	uint64_t t0 = GetTimeBase();

	for (int i = 0; i < iters; i++) {
	    optiq_pami_transport_rput(local_buf, nbytes, local_rank, remote_buf, remote_rank);
	}

	uint64_t t1 = GetTimeBase();

	double max_t, t = (double)(t1 - t0)/1.6e3/iters;

	MPI_Reduce(&t, &max_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (pami_transport->rank == 0) {
	    double bw = (double)nbytes/1024/1024/max_t*1e6;
	    printf("nbytes = %d t = %8.4f(us) bw = %8.4f(MB/s)\n", nbytes, max_t, bw);
	}
    }

    if (pami_transport->rank == remote_rank)
    {
	if (memcmp(local_buf, remote_buf, rput_bytes) != 0) {
	    printf("Invalid data recv\n");
	} else {
	    printf("Valid data recv\n");
	}
    }

    free(local_buf);
    free(remote_buf);

    optiq_pami_transport_finalize();

    return 0;
}
