#include <stdlib.h>
#include <mpi.h>
#include "test.h"

void ring_warm_up(int iters)
{
    int world_rank, world_size;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int buf_size = 8 * 1024 * 1024;
    void *send_buf = malloc(buf_size);
    void *recv_buf = malloc(buf_size);

    int next = (world_rank + 1) % world_size;
    int pre = (world_rank + world_size - 1) % world_size;

    MPI_Request request;
    MPI_Status status;

    for (int i = 0; i < iters; i++) {
        MPI_Barrier(MPI_COMM_WORLD);

        MPI_Irecv(recv_buf, buf_size, MPI_BYTE, pre, 0, MPI_COMM_WORLD, &request);
        MPI_Send(send_buf, buf_size, MPI_BYTE, next, 0, MPI_COMM_WORLD);

        MPI_Wait(&request, &status);
    }

    free(send_buf);
    free(recv_buf);
}

