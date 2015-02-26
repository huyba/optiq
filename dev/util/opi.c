#include "opi.h"
#include <mpi.h>

struct optiq_performance_index opi;

void optiq_opi_collect(int world_rank)
{
    struct optiq_performance_index max_opi;

    MPI_Reduce (&opi.transfer_time, &max_opi.transfer_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.recv_len, &max_opi.recv_len, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        double max_time =  max_opi.transfer_time / opi.iters;
        double bw = (double) max_opi.recv_len / max_time / 1024 / 1024 * 1e6;
        printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", max_opi.recv_len/1024/1024, max_time, bw);
    }
}
