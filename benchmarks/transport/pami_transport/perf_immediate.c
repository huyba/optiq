#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <mpi.h>

#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>

#include "pami_transport.h"

#define OPTIQ_TEST_PAMI_IMM 100

void optiq_recv_test_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    int *val = (int *)cookie;
    (*val)--;

    /*printf("rank %d val = %d\n", pami_transport->rank, *val);*/
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    optiq_pami_transport_init();
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int rank = pami_transport->rank;

    int local_rank = 1;
    int imm_bytes = 128;
    void *local_buf = malloc (imm_bytes);

    for (int i = 0; i < imm_bytes; i++) {
	((char*)local_buf)[i] = i % 128;
    }

    int remote_rank = 3;

    int iters = 30;
    int cookie = iters;

    pami_dispatch_callback_function fn;
    pami_dispatch_hint_t options = {};

    /*Receive memory request*/
    fn.p2p = optiq_recv_test_fn;
    pami_result_t result = PAMI_Dispatch_set (pami_transport->context,
            OPTIQ_TEST_PAMI_IMM,
            fn,
            (void *) &cookie,
            options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return 0;
    }

    for (int nbytes = 1; nbytes <= imm_bytes; nbytes++)
    {
	MPI_Barrier(MPI_COMM_WORLD);

	uint64_t t0 = GetTimeBase();

	if (rank == local_rank) 
	{
	    for (int i = 0; i < iters; i++) {
		optiq_pami_send_immediate(pami_transport->context, OPTIQ_TEST_PAMI_IMM, NULL, 0, local_buf, nbytes, pami_transport->endpoints[remote_rank]);
	    }

	    while (cookie > 0) {
		PAMI_Context_advance (pami_transport->context, 100);
	    }
	}

	if (rank == remote_rank) 
	{
	    while (cookie > 0) {
                PAMI_Context_advance (pami_transport->context, 100);
            }

	    for (int i = 0; i < iters; i++) {
                optiq_pami_send_immediate(pami_transport->context, OPTIQ_TEST_PAMI_IMM, NULL, 0, local_buf, nbytes, pami_transport->endpoints[local_rank]);
            }
	}

	cookie = iters;

	uint64_t t1 = GetTimeBase();

	double max_t, t = (double)(t1 - t0)/1.6e3/iters;

	MPI_Reduce(&t, &max_t, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

	if (pami_transport->rank == 0) {
	    max_t = max_t/2;
	    double bw = (double)nbytes/1024/1024/max_t*1e6;
	    printf("nbytes = %d t = %8.4f(us) bw = %8.4f(MB/s)\n", nbytes, max_t, bw);
	}
    }

    free(local_buf);

    optiq_pami_transport_finalize();

    return 0;
}
