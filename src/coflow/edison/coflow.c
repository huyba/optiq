#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"
#include "time.h"

#include "mpi.h"

#define BILLION  1000000000L;

int main(int argc, char **argv) {
    int myrank, numprocs;

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    int num_dims = 3;
    int coord[3];
    int size[3];

    int myId = myrank;
    int centerId = numprocs/2;

    int neighbors[16];
    int num_neighbors = 15;
    for (int i = 0; i < numprocs/2; i ++) {
	neighbors[i] = i;
    }
    for (int i = numprocs/2; i < numprocs-1; i ++) {
        neighbors[i] = i+1;
    }

    MPI_File fh;
    char fileName[] = "temp_test";
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fh);

    if (myrank == 0)
	printf("File opened\n");

    int write_count = 256*1024*1024;
    void *write_buf = malloc(write_count);
    int send_count = 8*1024*1024;
    char *send_buf = (char*)malloc(send_count);
    MPI_Status w_status;
    MPI_Offset offset = 0;

    int iters = 100;
    MPI_Request **requests = (MPI_Request**)malloc(sizeof(MPI_Request*)*iters);
    MPI_Status **status = (MPI_Status**)malloc(sizeof(MPI_Status*)*iters);
    for (int i = 0; i < iters; i++) {
	requests[i] = (MPI_Request*)malloc(sizeof(MPI_Request)*num_neighbors);
	status[i] = (MPI_Status*)malloc(sizeof(MPI_Status)*num_neighbors);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (myrank == 0) {
	printf("Start testing\n");
    }

    if	(myId == centerId) {
	printf("Rank %d: I'm center. Neighbors: ", myId);
	for (int i = 0; i < num_neighbors; i++) {
	    printf("%d ", neighbors[i]);
	}
	printf("\n");
    }

    for (int i = 0; i < num_neighbors; i++) {
	if (myId == neighbors[i]) {    
	    printf("I'm Neighbor %d\n", neighbors[i]);
	}
    }

    /*Do warm up - 100 times comm and 5 times I/O*/
    

    struct timespec start, end;

    /*Test 1: Comm only*/
    MPI_Barrier(MPI_COMM_WORLD);
    if (myId == 0) {
	printf("\nTest 1: Comm only between center and its neighbors\n\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (myId == centerId)
    {
        /*Post MPI_Isend*/
        for (int i = 0; i < iters; i++) {
            for (int j = 0; j < num_neighbors; j++) {
                int dest = neighbors[j];
                MPI_Isend(send_buf, send_count, MPI_BYTE, dest, 0, MPI_COMM_WORLD, &requests[i][j]);
            }
        }

        /*Check if the request is done*/
        for (int i = 0; i < iters; i++) {
            MPI_Waitall(num_neighbors, requests[i], status[i]);
        }
    }

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < iters; i++) {
        for(int j = 0; j < num_neighbors; j++) {
            if (myId == neighbors[j]) {
                MPI_Recv(send_buf, send_count, MPI_BYTE, centerId, 0, MPI_COMM_WORLD, &status[i][0]);
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);

    
    double elapsed = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
    elapsed = elapsed/iters;

    double max_elapsed;
    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, centerId, MPI_COMM_WORLD);
    double bw = 0;

    if (myId == centerId) {
	bw = (double)send_count/1024/1024/max_elapsed;
        printf("Comm Flow Only: Elapsed time at receiving side = %8.6f bw = %8.4f\n", max_elapsed, bw);
    }

    for (int i = 0; i < num_neighbors; i++) {
        if  (myId == neighbors[i]) {
            bw = (double)send_count/1024/1024/elapsed;
            printf("Neighbor %d Comm Flow Only: Elapsed time = %8.6f bw = %8.4f\n", myId, elapsed, bw);
        }
    }

    /*Test 2: I/O only*/
    MPI_Barrier(MPI_COMM_WORLD);
    if (myId == 0) {
	printf("\nTest 2: I/O only from center.\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fh);
    MPIO_Request *write_requests = (MPIO_Request*)malloc(sizeof(MPIO_Request)*iters);
    MPI_Status *write_status = (MPI_Status*)malloc(sizeof(MPI_Status)*iters);

    MPI_Barrier(MPI_COMM_WORLD);

    clock_gettime(CLOCK_REALTIME, &start);

    if (myId == centerId)
    {
        /*Post MPI_Isend*/
        for (int i = 0; i < iters; i++) {
	    MPI_File_write_at(fh, offset, write_buf, write_count, MPI_BYTE, &write_status[i]);
	    offset += write_count;
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);

    elapsed = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
    elapsed = elapsed/iters;

    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, centerId, MPI_COMM_WORLD);

    if (myId == centerId) {
	bw = (double)send_count/1024/1024/max_elapsed;
        printf("I/O Flow Only: Elapsed time = %8.6f bw = %8.4f\n", max_elapsed, bw);
    }

    /*Test 3: Post Isend, I/O, Waitall for Isend*/
    MPI_Barrier(MPI_COMM_WORLD);
    if (myId == 0) {
        printf("\nTest 3: Post Isend, I/O, Waitall for Isend\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);

    if (myId == centerId) {
	/*Post MPI_Isend*/
	for (int i = 0; i < iters; i++) {
	    for (int j = 0; j < num_neighbors; j++) {
		int dest = neighbors[j];
		MPI_Isend(send_buf, send_count, MPI_BYTE, dest, 0, MPI_COMM_WORLD, &requests[i][j]);
	    }
	}

	/*Do I/O*/
	for (int i = 0; i < iters; i ++) {
	    MPI_File_write_at(fh, offset, write_buf, write_count, MPI_BYTE, &w_status);
	    offset += write_count;
	}

	/*Check if the request is done*/
	for (int i = 0; i < iters; i++) {
	    MPI_Waitall(num_neighbors, requests[i], status[i]);
	}
    }

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < iters; i++) {
	for(int j = 0; j < num_neighbors; j++) {
	    if (myId == neighbors[j]) {
		MPI_Recv(send_buf, send_count, MPI_BYTE, centerId, 0, MPI_COMM_WORLD, &status[i][0]);
	    }
	}
    }

    clock_gettime(CLOCK_REALTIME, &end);
    
    MPI_File_close(&fh);

    elapsed = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
    elapsed = elapsed/iters;

    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, centerId, MPI_COMM_WORLD);

    if (myId == centerId) {
	bw = (double)send_count/1024/1024/max_elapsed;
	printf("CoFlow - Comm first: Elapsed time at receiving side = %8.6f bw = %8.4f\n", max_elapsed, bw);
    }

    for	(int i = 0; i < num_neighbors; i++) {
	if  (myId == neighbors[i]) {
	    bw = (double)send_count/1024/1024/elapsed;
	    printf("Neighbor %d CoFlow - Comm first: Elapsed time = %8.6f bw = %8.4f\n", myId, elapsed, bw);
	}
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (myId == 0) {
	printf("\nTest 4: iwrite, isend, wait all iwrite, wait all isend\n");
    }
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fh);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myId == centerId)
    {
	/*Do I/O*/
        for (int i = 0; i < iters; i ++) {
            MPI_File_iwrite_at(fh, offset, write_buf, write_count, MPI_BYTE, &write_requests[i]);
            offset += write_count;
        }

        /*Post MPI_Isend*/
        for (int i = 0; i < iters; i++) {
            for (int j = 0; j < num_neighbors; j++) {
                int dest = neighbors[j];
                MPI_Isend(send_buf, send_count, MPI_BYTE, dest, 0, MPI_COMM_WORLD, &requests[i][j]);
            }
        }

	MPI_Waitall(iters, write_requests, write_status);

        /*Check if the request is done*/
        for (int i = 0; i < iters; i++) {
            MPI_Waitall(num_neighbors, requests[i], status[i]);
        }
    }

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < iters; i++) {
        for (int j = 0; j < num_neighbors; j++) {
            if (myId == neighbors[j]) {
                MPI_Recv(send_buf, send_count, MPI_BYTE, centerId, 0, MPI_COMM_WORLD, &status[i][0]);
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);

    MPI_File_close(&fh);

    elapsed = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
    elapsed = elapsed/iters;

    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, centerId, MPI_COMM_WORLD);
    
    if (myId == centerId) {
	bw = (double)send_count/1024/1024/max_elapsed;
        printf("CoFlow - I/O first: Elapsed time at receiving side = %8.6f bw = %8.4f\n", max_elapsed, bw);
    }

    for (int i = 0; i < num_neighbors; i++) {
        if (myId == neighbors[i]) {
            bw = (double)send_count/1024/1024/elapsed;
            printf("Neighbor %d CoFlow - I/O first: Elapsed time = %8.6f bw = %8.4f\n", myId, elapsed, bw);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (myId == 0) {
        printf("\nTest 5: isend, iwrite, wait all isend, wait all iwrite\n");
    }
    MPI_File_open(MPI_COMM_WORLD, fileName, MPI_MODE_RDWR | MPI_MODE_CREATE | MPI_MODE_DELETE_ON_CLOSE, MPI_INFO_NULL, &fh);

    MPI_Barrier(MPI_COMM_WORLD);

    if (myId == centerId)
    {
        /*Post MPI_Isend*/
        for (int i = 0; i < iters; i++) {
            for (int j = 0; j < num_neighbors; j++) {
                int dest = neighbors[j];
                MPI_Isend(send_buf, send_count, MPI_BYTE, dest, 0, MPI_COMM_WORLD, &requests[i][j]);
            }
        }
	
        /*Do I/O*/
        for (int i = 0; i < iters; i ++) {
            MPI_File_iwrite_at(fh, offset, write_buf, write_count, MPI_BYTE, &write_requests[i]);
            offset += write_count;
        }

	/*Check if the request is done*/
        for (int i = 0; i < iters; i++) {
            MPI_Waitall(num_neighbors, requests[i], status[i]);
        }

	MPI_Waitall(iters, write_requests, write_status);
    }

    clock_gettime(CLOCK_REALTIME, &start);

    for (int i = 0; i < iters; i++) {
        for (int j = 0; j < num_neighbors; j++) {
            if (myId == neighbors[j]) {
                MPI_Recv(send_buf, send_count, MPI_BYTE, centerId, 0, MPI_COMM_WORLD, &status[i][0]);
            }
        }
    }

    clock_gettime(CLOCK_REALTIME, &end);

    MPI_File_close(&fh);

    elapsed = (end.tv_sec - start.tv_sec) + (double)(end.tv_nsec - start.tv_nsec)/(double)BILLION;
    elapsed = elapsed/iters;

    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, centerId, MPI_COMM_WORLD);
    bw = (double)send_count/1024/1024/max_elapsed;

    if (myId == centerId) {
        printf("CoFlow: Isend, iwrite, wait isend, wait i/0 time = %8.6f bw = %8.4f\n", max_elapsed, bw);
    }

    for (int i = 0; i < num_neighbors; i++) {
        if (myId == neighbors[i]) {
            bw = (double)send_count/1024/1024/elapsed;
            printf("Neighbor %d isend, iwrite, wait isend, wait i/o: Elapsed time = %8.6f bw = %8.4f\n", myId, elapsed, bw);
        }
    }
    MPI_Finalize();
}
