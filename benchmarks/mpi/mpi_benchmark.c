#include <mpi.h>
#include <sys/time.h>

double mpi_time = 0;

void gather_and_print_time (timeval t0, timeval t1, int iters, long int recv_len, int world_rank)
{
    double elapsed_time = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);

    double max_time = 0.0;

    MPI_Reduce (&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    long int total_length = 0L;

    MPI_Reduce(&recv_len, &total_length, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
	max_time = max_time / iters;
	mpi_time = max_time;
	double bw = (double) total_length / max_time / 1024 / 1024 * 1e6;
	printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", total_length/1024/1024, max_time, bw);
    }
}

void optiq_benchmark_mpi_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls)
{
    timeval t0, t1;

    int iters = 10;

    MPI_Barrier (MPI_COMM_WORLD);

    gettimeofday(&t0, NULL);

    for (int i = 0; i < iters; i++) {
	MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_BYTE, recvbuf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    gettimeofday(&t1, NULL);
    
    MPI_Barrier(MPI_COMM_WORLD);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if (world_rank == 0) {
	printf("MPI_Alltoallv ");
    }

    int recv_len = 0;
    for (int i = 0; i < world_size; i++) {
	recv_len += recvcounts[i];
    }

    gather_and_print_time (t0, t1, iters, recv_len, world_rank);
}
