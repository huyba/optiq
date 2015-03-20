#ifndef OPTIQ_BENCHMARK_MPI
#define OPTIQ_BENCHMARK_MPI

extern "C" double mpi_time;

void optiq_benchmark_mpi_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls);

#endif
