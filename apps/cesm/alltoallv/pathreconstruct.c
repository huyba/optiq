#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "datagen.h"
#include "topology.h"

void reconstruct_paths(int num_dims, int *size, int *torus)
{
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }
    
    int **all_coords = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
	all_coords[i] = (int *)malloc(sizeof(int) * num_dims);
	for (int j = 0; j < num_dims; j++) {
	    all_coords[i][j] = 0;
	}
    }

    int coord[5], nid = 0;
    for (int ad = 0; ad < size[0]; ad++) {
        coord[0] = ad;
        for (int bd = 0; bd < size[1]; bd++) {
            coord[1] = bd;
            for (int cd = 0; cd < size[2]; cd++) {
                coord[2] = cd;
                for (int dd = 0; dd < size[3]; dd++) {
                    coord[3] = dd;
                    for (int ed = 0; ed < size[4]; ed++) {
                        coord[4] = ed;
			nid = optiq_compute_nid(num_dims, size, coord);
			for (int i = 0; i < num_dims; i++) {
			    all_coords[nid][i] = coord[i];
			}
		    }
		}
	    }
        }
    }

    int **graph = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
	graph[i] = (int *)malloc(sizeof(int) * num_nodes);
	for (int j = 0; j < num_nodes; j++) {
	    graph[i][j] = 0;
	}
    }

    int order[5];

    optiq_topology_compute_routing_order_bgq(num_dims, size, order);

    printf("Order [%d %d %d %d %d]\n", order[0], order[1], order[2], order[3], order[4]);

    int max_hops = 20;
    int **path = (int **)malloc(sizeof(int *) * max_hops);

    for (int i = 0; i < max_hops; i++) {
	path[i] = (int *)malloc(sizeof(int) * num_dims);
	for (int j = 0; j < num_dims; j++) {
	    path[i][j] = 0;
	}
    }

    int num_hops, u, v;
    for (int dest_id = 32; dest_id < num_nodes; dest_id += 64) {
	for (int ad = 0; ad < size[0]; ad++) {
	    coord[0] = ad;
	    for (int bd = 0; bd < size[1]; bd++) {
		coord[1] = bd;
		for (int cd = 0; cd < size[2]; cd++) {
		    coord[2] = cd;
		    for (int dd = 0; dd < size[3]; dd++) {
			coord[3] = dd;
			for (int ed = 0; ed < size[4]; ed++) {
			    coord[4] = ed;

			    optiq_topology_reconstruct_path_bgq(num_dims, size, torus, order, coord, all_coords[dest_id], path);
			    num_hops = optiq_compute_num_hops(num_dims, coord, all_coords[dest_id]);
			    //printf("Done constructing path, #hops = %d\n", num_hops);

			    for (int h = 0; h < num_hops; h ++) {
				u = optiq_compute_nid(num_dims, size, path[h]);
				v = optiq_compute_nid(num_dims, size, path[h+1]);
				graph[u][v]++;
			    }
			}
                    }
                }
            }
        }
    }

    int max_paths = 0;
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
	    if (graph[i][j] != 0) {
		printf("%d %d %d\n", i, j, graph[i][j]);
	    }

            if (max_paths < graph[i][j]) {
		max_paths = graph[i][j];
		u = i;
		v = j;
	    }
        }
    }

    printf("Max #paths/link (%d, %d) = %d\n", u, v, max_paths);
}

int main(int argc, char **argv)
{
    int num_dims = 5;
    int size[5] = {2, 4, 4, 8, 2};
    int torus[5] = {0, 1, 1, 1, 1};

    reconstruct_paths(num_dims, size, torus);

    return 0;   
}
