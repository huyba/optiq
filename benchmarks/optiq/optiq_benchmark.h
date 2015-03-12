#ifndef OPTIQ_BENCHMARK
#define OPTIQ_BENCHMARK

void optiq_benchmark_pattern_from_file (char *filepath, int rank, int size);

void optiq_benchmark_reconstruct_mpi_paths(int *sendcounts, std::vector<struct path *> &mpi_paths);

#endif
