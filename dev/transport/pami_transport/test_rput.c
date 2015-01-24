#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    optiq_pami_transport_init();

    int local_rank = 0;
    int rput_bytes = 1024 * 1024;
    void *local_buf = malloc (rput_bytes);

    for (int i = 0; i < rput_bytes; i++) {
	((char*)local_buf)[i] = i % 128;
    }

    int remote_rank = 1;
    void *remote_buf = malloc (rput_bytes);

    optiq_pami_transport_rput(local_buf, rput_bytes, local_rank, remote_buf, remote_rank);

    if (pami_transport->rank == remote_rank)
    {
	if (memcmp(local_buf, remote_buf, rput_bytes) != 0) {
	    printf("Invalid data recv\n");
	} else {
	    printf("Valid data recv\n");
	}
    }

    return 0;
}
