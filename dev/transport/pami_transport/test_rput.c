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
    optiq_pami_transport_init();

    int local_rank = 0;
    int rput_bytes = 128 * 1024 * 1024;
    void *local_buf = malloc (rput_bytes);

    for (int i = 0; i < rput_bytes; i++) {
	((char*)local_buf)[i] = i % 128;
    }

    int remote_rank = 1;
    void *remote_buf = malloc (rput_bytes);

    for (int nbytes = 1024; nbytes < rput_bytes; nbytes *= 2)
    {
	optiq_pami_transport_rput(local_buf, nbytes, local_rank, remote_buf, remote_rank);

	if (pami_transport->rank == remote_rank)
	{
	    if (memcmp(local_buf, remote_buf, nbytes) != 0) {
		printf("Invalid data recv\n");
	    } else {
		printf("Valid data recv\n");
	    }
	}

    }

    free(local_buf);
    free(remote_buf);

    optiq_pami_transport_finalize();

    return 0;
}
