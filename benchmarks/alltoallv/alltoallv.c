#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include <hwi/include/bqc/A2_inlines.h>

int main(int argc, char **argv) 
{
    MPI_Init(&argc, &argv);

    int world_size, world_rank;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int nbytes = 1*1024*1024;

    if (argc > 1) {
	nbytes = atoi(argv[1]);
    }

    int ratio = 64;
    int num_dests = world_size/ratio;
    
    void *sendbuf = malloc(nbytes * num_dests);
    void *recvbuf = malloc(nbytes * world_size);

    int *sendcounts = (int *)malloc(sizeof(int) * world_size);
    int *sdispls = (int *)malloc(sizeof(int) * world_size);
    int *recvcounts = (int *)malloc(sizeof(int) * world_size);
    int *rdispls = (int *)malloc(sizeof(int) * world_size);

    for (int i = 0; i < world_size; i++) {
	sendcounts[i] = 0;
	sdispls[i] = 0;
	recvcounts[i] = 0;
	rdispls[i] = 0;
    }

    int dest, source;
    /*if (0 <= world_rank && world_rank < world_size/8) {
	dest = world_rank + world_size/8 * 7;
	sendcounts[dest] = nbytes;
	printf("Rank %d sends data to rank %d\n", world_rank, dest);
    }

    if (world_size/8 * 7 <= world_rank && world_rank < world_size) {
	source = world_rank - world_size/8 * 7;
	recvcounts[source] = nbytes;
	printf("Rank %d recvs data to rank %d\n", world_rank, source);
    }*/

    /*At sending side*/
    for (int i = 0; i < num_dests; i++) {
	dest = i * ratio + ratio/2;
        sendcounts[dest] = nbytes;
        sdispls[i] = i * nbytes ;
    }

    /*At receiving side*/
    if (world_rank % ratio == ratio/2) {
	printf("receiving rank %d\n", world_rank);

	for (int i = 0; i < world_size; i++) {
	    recvcounts[i] = nbytes;
	    rdispls[i] = i * nbytes;
	}
    }

    int iters = 30;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int i = 0; i < iters; i++) {
	MPI_Alltoallv(sendbuf, sendcounts, sdispls, MPI_BYTE, recvbuf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t end = GetTimeBase();

    double elapsed_time = (double)(end - start)/1.6e3;
    double max_time = 0.0;

    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
	max_time = max_time/iters;
	double bw = num_dests * world_size * nbytes/max_time/1024/1024*1e6;
	printf("t = %8.4f, bw = %8.4f\n", max_time, bw);
    }

    MPI_Finalize();
}
