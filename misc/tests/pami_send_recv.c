#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "optiq.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct optiq_transport transport;
    optiq_transport_init(&transport, PAMI);

    if (world_rank == 0) {
	printf("Init transport successfully!\n");
    }

    int data_size = 4*1024*1024;
    char *buffer = (char *)malloc(data_size);

    struct optiq_message *message = get_message_with_buffer(data_size);
    message->header.flow_id = 0;
    message->header.final_dest = 1;
    message->header.original_length = data_size;
    message->header.original_offset = 0;
    message->header.original_source = 0;
    message->length = data_size;

    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(&transport);

    int num_iters = 1;
    if (argc > 1) {
	num_iters = atoi(argv[1]);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /*Iterate the arbitration table to get the next virtual lane*/
    uint64_t start = GetTimeBase();

    for (int i = 0; i < num_iters; i++) {
	if (world_rank == 0) {
	    message->next_dest = 1;

	    optiq_transport_send(&transport, message);

	    bool isDone = false;

	    while (!isDone) {
		PAMI_Context_advance (pami_transport->context, 100);
		if (pami_transport->in_use_send_cookies.size() > 0) {
		    isDone = true;
		    pami_transport->in_use_send_cookies.pop_back();
		}
	    }
	}

	if ( world_rank == 1) {
	    int isDone = 0;
	    message->recv_length = 0;
	    while (isDone == 0) {
		isDone = optiq_transport_recv(&transport, message);
	    }
	}
    }

    uint64_t end = GetTimeBase();

    double elapsed_time = (double)(end-start)/1.6e3;
    double max_time;
    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double bw = (double) data_size * 1e6 / 1024 / 1024 / elapsed_time / num_iters;
    printf("Elapse time = %8.0f, bw = %8.4f\n", elapsed_time, bw);

    /*if (world_rank == 0) {
     *         double bw = (double) data_size * 1e6 / 1024 / 1024 / max_time / num_iters;
     *                 printf("Elapse time = %8.0f, bw = %8.4f\n", max_time, bw);
     *                     }*/

    printf("Rank %d completed the test successfully\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Status status;

    start = GetTimeBase();

    if (world_rank == 0) {
	MPI_Send(buffer, data_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
    }

    if (world_rank == 1) {
	MPI_Recv(buffer, data_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
    }

    end = GetTimeBase();

    elapsed_time = (double)(end-start)/1.6e3;

    bw = (double) data_size * 1e6 / 1024 / 1024 / elapsed_time / num_iters;
    printf("MPI send/recv Elapse time = %8.0f, bw = %8.4f\n", elapsed_time, bw);


    MPI_Finalize();

    return 0;
}
