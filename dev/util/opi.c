#include "opi.h"
#include <mpi.h>

struct optiq_performance_index opi;

void optiq_opi_collect(int world_rank)
{
    struct optiq_performance_index max_opi;

    MPI_Reduce (&opi.transfer_time, &max_opi.transfer_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.notification_done_time, &max_opi.notification_done_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.sendimm_time, &max_opi.sendimm_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Reduce (&opi.recv_len, &max_opi.recv_len, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
        double max_time =  max_opi.transfer_time / opi.iters;
        double bw = (double) max_opi.recv_len / max_time / 1024 / 1024 * 1e6;
        printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", max_opi.recv_len/1024/1024, max_time, bw);
	printf("notification done time is %8.4f\n", max_opi.notification_done_time);
	printf("send_immediate time is %8.4f\n", max_opi.sendimm_time);
    }
}

void optiq_opi_clear()
{
    opi.recv_len = 0;
    opi.sendimm_time  = 0;
    opi.notification_done_time = 0;
    opi.transfer_time = 0;
    opi.build_path_time = 0;
}
