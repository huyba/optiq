#include <stdio.h>

#include <datagen.h>
#include <topology.h>

#define INFINITY (8*1024*1024)

void optiq_print_arcs(int num_dims, int *size, double cap)
{
    int num_neighbors = 0;
    int neighbors[10];
    int coord[5];
    int nid;

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
                        num_neighbors = 0;
                        nid = optiq_compute_nid(num_dims, size, coord);
                        num_neighbors = optiq_compute_neighbors(num_dims, size, coord, neighbors);
                        for (int i = 0; i < num_neighbors; i++) {
                            if (cap < 0.0) {
                                printf("%d %d\n", nid, neighbors[i]);
                            }
                            else {
                                printf("%d %d %8.0f\n", nid, neighbors[i], cap);
                            }
                        }
			/*A node connects to itself*/
			if (cap < 0.0) {
                            printf("%d %d\n", nid, nid);
                        }
                        else {
                            printf("%d %d %8.0f\n", nid, nid, cap);
                        }    
                    }
                }
            }
        }
    }
}

void optiq_generate_mct_data(int num_dims, int *size, int problem_size)
{
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }

    printf("set Nodes :=\n");
    for (int i = 0; i < num_nodes; i++) {
	printf("%d\n", i);
    }
    printf(";\n\n");

    printf("set Arcs :=\n");
    double cap = -1.0;
    optiq_print_arcs(num_dims, size, cap);
    printf(";\n\n");

    cap = 2048.0;
    printf("param Capacity :=\n");
    optiq_print_arcs(num_dims, size, cap);
    printf(";\n\n");

    double demand = 2048.0;
    printf("param: Jobs: Source Destination Demand :=\n");
    int jobId = 0;

    for (int i = 0; i < problem_size/3; i++) {
	printf("%d %d %d %8.1f\n", jobId, i, i+problem_size*2/3, demand);
	jobId++;
    }
    for (int i = problem_size/3; i < problem_size*2/3; i++) {
	printf("%d %d %d %8.1f\n", jobId, i, i + problem_size/3, demand);
	jobId++;
    }

    printf(";");
}
void optiq_generate_dataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds)
{
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }

    printf("set Nodes :=\n");
    for (int i = 0; i < num_nodes; i++) {
	printf("%d\n", i);
    }
    for (int i = 0; i < num_bridges/2; i++) {
	printf("ION_%d\n", i);
    }
    printf("SuperION\n");
    printf(";\n\n");

    printf("set Arcs :=\n");
    double cap = -1.0;
    optiq_print_arcs(num_dims, size, cap);
    for (int i = 0; i < num_bridges; i++) {
	printf("%d ION_%d\n", bridgeIds[i], i/2);
    }
    for (int i = 0; i < num_bridges/2; i++) {
	printf("ION_%d SuperION\n", i);
    }
    printf(";\n\n");

    printf("set Types :=\n");
    printf("IO\n");
    printf("COMM\n");
    printf(";\n\n");

    cap = 2048.0;
    printf("param Capacity :=\n");
    optiq_print_arcs(num_dims, size, cap);
    for (int i = 0; i < num_bridges; i++) {
	printf("%d ION_%d %8.1f\n", bridgeIds[i], i/2, cap);
    }
    for (int i = 0; i < num_bridges/2; i++) {
	printf("ION_%d SuperION %d\n", i, INFINITY);
    }
    printf(";\n\n");

    printf("param Weight:=\n");
    printf("IO 0.5\n");
    printf("COMM 0.5\n");
    printf(";\n\n");

    double demand = 2048.0;
    int jobId = 0;
    printf("param: Jobs: Source Destination Demand Types :=\n");
    for (int i = 0; i < num_sources; i++) {
	for (int j = 0; j < factor; j++) {
	    printf("%d %d %d %8.1f COMM\n", jobId, i, num_nodes-num_sources*factor+i*factor+j, demand);
	    jobId++;
	}
    }
    for (int i = 0 ; i < num_sources; i++) {
	printf("%d %d SuperION %8.1f IO\n", jobId, i, demand);
    }
    printf(";");
}
