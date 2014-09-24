#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"

#include <hwi/include/bqc/A2_inlines.h>

#include "topology.h"

#include "mpi.h"

#define ATM 1
#define LND 2
#define ICE 3
#define OCN 4
#define CPL 5

int getNodeType(int world_rank, int phase, int *nodeType, int *dest) 
{
    if (phase == 0) {
	if (world_rank >= 0 && world_rank <= 31) {
	    *nodeType = ATM;
	}
	*dest = world_rank + 64;
    } else if (phase == 1) {
	if (world_rank >= 0 && world_rank <= 15) {
            *nodeType = LND;
        } else if (world_rank >= 16 && world_rank <= 31){
	    *nodeType = ICE;
	}
	*dest = world_rank + 64;
    }

    if (world_rank >= 32 && world_rank <= 63) {
	*nodeType = OCN;
	*dest = world_rank + 32;
    }

    if (world_rank >= 64 && world_rank <= 95) {
        *nodeType = CPL;
	*dest = world_rank;
    }
}

int main(int argc, char **argv) 
{
    int world_rank, world_size;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int phase = 0;
    int dest = 0;
    int nodeType = 0;
    getNodeType(world_rank, phase, &nodeType, &dest);

    int iters = 30;
    int dest = 0;

    int buf_size = 8*1024*1024;
    int buf1_size = 8*1024*1024;
    int buf2_size = 8*1024*1024;
    char *send_buf = (char*)malloc(buf_size);
    char *recv_buf1 = (char*)malloc(buf1_size);
    char *recv_buf2 = (char*)malloc(buf2_size);

    int module_size = 32;
    MPI_Status *status = (MPI_Status *)malloc(2*sizeof(MPI_Status)*module_size);
    MPI_Request *request = (MPI_Request *)malloc(2*sizeof(MPI_Request)*module_size);

    uint64_t start = GetTimeBase();

    int count = buf_size;
    if (nodeType == ATM || nodeType == OCN) {
	for (int i = 0; i < iters; i++) {
	    MPI_Isend(send_buf, count, MPI_BYTE, dest, 0, MPI_COMM_WORLD, &request[i]);
	}
	MPI_Waitall(iters, request, status);
    }

    if (nodeType == CPL) {
	int source1 = world_rank - 64;
	int source2 = world_rank - 32;
	for (int i = 0; i < iters; i++) {
	    MPI_Irecv(recv_buf1, count, MPI_BYTE, source1, 0, MPI_COMM_WORLD, &request[i*2]);
	    MPI_Irecv(recv_buf2, count, MPI_BYTE, source2, 0, MPI_COMM_WORLD, &request[i*2+1]);
	}
	MPI_Waitall(iters*2, request, status);
    }

    uint64_t end = GetTimeBase();

    double elapsed = (double)(end-start)/1.6e3/iters;
    double max_elapsed;
    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    double bw = (double)count/1024/1024/max_elapsed*1e6;

    if (world_rank == 0) {
        printf("Max elapsed time = %8.0f bw = %8.4f\n", max_elapsed, bw);
    }

    MPI_Finalize();
}
