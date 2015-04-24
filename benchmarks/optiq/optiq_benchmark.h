#ifndef OPTIQ_BENCHMARK
#define OPTIQ_BENCHMARK

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size);

void optiq_benchmark_mpi_perf(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls);

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths);

void optiq_benchmark_jobs_from_file (char *jobfile, int datasize);

#endif
