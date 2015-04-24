#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    optiq_pami_transport_init();

    int send_rank = 0;
    int send_bytes = 1024 * 1024;
    void *send_buf = malloc (send_bytes);

    for (int i = 0; i < send_bytes; i++) {
	((char*)send_buf)[i] = i % 128;
    }

    int recv_rank = 1;
    int recv_bytes = 1024 * 1024;
    void *recv_buf = malloc (recv_bytes);

    if (pami_transport->rank == send_rank) 
    {
	optiq_pami_transport_send(send_buf, send_bytes, recv_rank);
    }

    if (pami_transport->rank == recv_rank)
    {
	optiq_pami_transport_recv(recv_buf, recv_bytes, send_rank);
    }

    if (pami_transport->rank == recv_rank)
    {
	if (memcmp(send_buf, recv_buf, send_bytes) != 0) {
	    printf("Invalid data recv\n");
	} else {
	    printf("Valid data recv\n");
	}
    }

    free(send_buf);
    free(recv_buf);

    optiq_pami_transport_finalize();

    return 0;
}
