#include "stdio.h"
#include "stdlib.h"
#include "stddef.h"

#ifdef __bgq__
#include <hwi/include/bqc/A2_inlines.h>
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "topology.h"
#include "datagen.h"

#include "mpi.h"

#define ATM 1
#define LND 2
#define ICE 3
#define OCN 4
#define CPL 5

int get_node_type(int world_rank, int phase, int *nodeType, int *dest) 
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
    get_node_type(world_rank, phase, &nodeType, &dest);

    int iters = 30;

    int buf_size = 8*1024*1024;
    int buf1_size = 8*1024*1024;
    int buf2_size = 8*1024*1024;
    char *send_buf = (char*)malloc(buf_size);
    char *recv_buf1 = (char*)malloc(buf1_size);
    char *recv_buf2 = (char*)malloc(buf2_size);

    int module_size = 32;
    MPI_Status *status = (MPI_Status *)malloc(2 * sizeof(MPI_Status) * module_size);
    MPI_Request *request = (MPI_Request *)malloc(2 * sizeof(MPI_Request) * module_size);

    int num_dims = 5;
    int source_coord[5], size[5], torus[5], dest_coord[5], order[5];

    optiq_get_topology_info(source_coord, size, torus);
    if (world_rank == 0) {
	printf("Size: %d x %d x %d x %d x %d Torus: %d %d %d %d %d\n", size[0], size[1], size[2], size[3], size[4], torus[0], torus[1], torus[2], torus[3], torus[4]);
    }

    /*printf("Rank %d coord [%d, %d, %d, %d, %d]\n", world_rank, source_coord[0], source_coord[1], source_coord[2], source_coord[3], source_coord[4]);*/

    BG_CoordinateMapping_t *all_coords = (BG_CoordinateMapping_t *) malloc(sizeof(BG_CoordinateMapping_t)*world_size);
    optiq_map_ranks_to_coords(all_coords, world_size);

    dest_coord[0] = all_coords[dest].a;
    dest_coord[1] = all_coords[dest].b;
    dest_coord[2] = all_coords[dest].c;
    dest_coord[3] = all_coords[dest].d;
    dest_coord[4] = all_coords[dest].e;

    optiq_compute_routing_order(num_dims, size, order);

    int num_hops = 0, **path, *nodes_on_path;
    char str[1024];
    if (nodeType == ATM || nodeType == OCN) {
	num_hops = optiq_compute_num_hops(num_dims, source_coord, dest_coord);
	path = (int **)malloc(sizeof(int*) * (num_hops + 1));
	nodes_on_path = (int *)malloc(sizeof(int) * (num_hops + 1));
	for (int i = 0; i < (num_hops + 1); i++) {
	    path[i] = (int*) malloc(sizeof(int) * num_dims);
	}
	optiq_reconstruct_path(num_dims, size, source_coord, dest_coord, order, torus, path);
	for (int i = 0; i < (num_hops + 1); i ++) {
	    optiq_coord_to_nodeId(num_dims, size, path[i], &nodes_on_path[i]);
	    //printf("Path from rank %d to %d node %d/%d: [%d, %d, %d, %d, %d]\n", world_rank, dest, i, num_hops + 1, path[i][0], path[i][1], path[i][2], path[i][3], path[i][4]);
	    /*printf("Path from rank %d to %d node %d/%d: %d\n", world_rank, dest, i, num_hops + 1, nodes_on_path[i]);*/
	}
    }

    int *all_hops = (int*)malloc(sizeof(int)*world_size);
    MPI_Allgather(&num_hops, 1, MPI_INT, all_hops, 1, MPI_INT, MPI_COMM_WORLD);
    int num_paths = 0;
    for (int i = 1; i < world_size; i++) {
	if(all_hops[i] != 0) {
	    num_paths++;
	}
    }

    if (num_hops != 0 && world_rank != 0) {
	MPI_Send(nodes_on_path, num_hops+1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }

    if (world_rank == 0) {
	int *path_length = (int *)malloc(sizeof(int) * num_paths);
	int **all_paths = (int **)malloc(sizeof(int*) * num_paths);
	MPI_Request *requests = (MPI_Request *)malloc(sizeof(MPI_Request) * num_paths);
	MPI_Status *status = (MPI_Status *)malloc(sizeof(MPI_Status) * num_paths);
	int j = 0;
	for (int i = 1; i < world_size; i++) {
	    if (all_hops[i] != 0) {
		path_length[j] = all_hops[i] + 1;
		all_paths[j] = (int *)malloc(sizeof(int) * path_length[j]);
		MPI_Irecv(all_paths[j], path_length[j], MPI_INT, i, 0, MPI_COMM_WORLD, &requests[j]);
		j++;
	    }
	}

	MPI_Waitall(num_paths, requests, status);

	int **link_contention  = (int **)malloc(sizeof(int*) * world_size);
	for (int i = 0; i < world_size; i++) {
	    link_contention[i] = (int *)malloc(sizeof(int) * world_size);
	    for (int j = 0; j < world_size; j++) {
		link_contention[i][j] = 0;
	    }
	}

	for (int i = 0; i < num_paths; i++) {
	    for(int j = 0; j < path_length[i] - 1; j++) {
		link_contention[all_paths[i][j]][all_paths[i][j+1]]++;
	    }
	}

	for (int i = 0; i < world_size; i++) {
	    for (int j = 0; j < world_size; j++) {
		if (link_contention[i][j] != 0) {
		    //printf("%d->%d : %d\n", i, j, link_contention[i][j]);
		}
	    }
	}

	optiq_generate_mct_data(num_dims, size, 96);
    }

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
	    MPI_Irecv(recv_buf1, count, MPI_BYTE, source1, 0, MPI_COMM_WORLD, &request[i * 2]);
	    MPI_Irecv(recv_buf2, count, MPI_BYTE, source2, 0, MPI_COMM_WORLD, &request[i * 2 + 1]);
	}
	MPI_Waitall(iters*2, request, status);
    }

    uint64_t end = GetTimeBase();

    double elapsed = (double)(end-start)/1.6e3/iters;
    double max_elapsed;
    MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    double bw = (double)count/1024/1024/max_elapsed * 1e6;

    if (world_rank == 0) {
        printf("Max elapsed time = %8.0f bw = %8.4f\n", max_elapsed, bw);
    }

    MPI_Finalize();
}
