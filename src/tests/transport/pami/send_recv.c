#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "pami_transport.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct optiq_pami_transport pami_transport;
    optiq_pami_transport_init(&pami_transport);

    if (world_rank == 0) {
	printf("Init transport successfully!\n");
    }

    int data_size = 4*1024*1024;
    if (argc > 1) {
	data_size = atoi(argv[1]) * 1024;
    }

    char *buffer = (char *)malloc(data_size);
    for (int i = 0; i < data_size; i++) {
	buffer[i] = i % 128;
    }

    int num_iters = 1;
    if (argc > 2) {
	num_iters = atoi(argv[2]);
    }

    if (world_rank == 0) {
	printf("num_iters = %d, data_size = %d\n", num_iters, data_size);
    }

    pami_transport.recv_cookie = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    /*Iterate the arbitration table to get the next virtual lane*/
    uint64_t start = GetTimeBase();
    
    for (int i = 0; i < num_iters; i++) {
	if (world_rank == 0) {
	    int cookie = 1;
	    int dest = 1;
	    pami_transport.involved_job_ids.push_back(0);

	    optiq_pami_transport_actual_send(&pami_transport, buffer, data_size, dest, &cookie);

	    bool isDone = false;

	    while (!isDone) {
		isDone = optiq_pami_transport_test(&pami_transport, &cookie);
	    }

	    while(!optiq_pami_transport_forward_test(&pami_transport));
	}

	if (world_rank == 1) {
	    struct optiq_message message;
	    message.length = data_size;
	    message.recv_length = 0;
	    pami_transport.recv_cookie++;
	    pami_transport.involved_task_ids.push_back(0);

	    int isDone = 0;
	    while (isDone == 0) {
		isDone = optiq_pami_transport_recv(&pami_transport, &message);
	    }
	}
	//printf("Rank %d done %dth iter\n", world_rank, i);
    }

    uint64_t end = GetTimeBase();

    double elapsed_time = (double)(end-start)/1.6e3;
    double max_time;
    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    double bw = (double) data_size * 1e6 / 1024 / 1024 / elapsed_time * num_iters;
    printf("Rank %d Elapse time = %8.0f, bw = %8.4f\n", world_rank, elapsed_time, bw);

    /*if (world_rank == 0) {
     *         double bw = (double) data_size * 1e6 / 1024 / 1024 / max_time / num_iters;
     *                 printf("Elapse time = %8.0f, bw = %8.4f\n", max_time, bw);
     *                     }*/

    printf("Rank %d completed the test successfully\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);
    MPI_Status status;

    start = GetTimeBase();

    for (int i = 0; i < num_iters; i++) {
	if (world_rank == 0) {
	   MPI_Send(buffer, data_size, MPI_BYTE, 1, 0, MPI_COMM_WORLD);
	}

	if (world_rank == 1) {
	    MPI_Recv(buffer, data_size, MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
	}
    }

    end = GetTimeBase();

    elapsed_time = (double)(end-start)/1.6e3;

    bw = (double) data_size * 1e6 / 1024 / 1024 / elapsed_time * num_iters;
    printf("MPI send/recv Elapse time = %8.0f, bw = %8.4f\n", elapsed_time, bw);

    MPI_Finalize();

    return 0;
}
