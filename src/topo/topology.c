#include <stdio.h>
#include <stdlib.h>
#include "topology.h"

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#define INFINITY (8*1024*1024)

int compute_nid(int num_dims, int *coord, int *size) {
    int nid = coord[num_dims-1];
    int  pre_size = 1;

    for (int i = num_dims-2; i >= 0; i--) {
        for (int j = i+1; j < num_dims; j++) {
            pre_size *= size[j];
	}

        nid += coord[i]*pre_size;
        pre_size = 1;
    }

    return nid;
}

int check_existing(int num_neighbors, int *neighbors, int nid) {
    for (int i = 0; i < num_neighbors; i++) {
        if (neighbors[i] == nid) {
            return 1;
        }
    }

    return 0;
}

int compute_neighbors(int num_dims, int *coord, int *size, int *neighbors) {
    int num_neighbors = 0;
    int nid = 0;

    for (int i = 0; i < num_dims; i++) {
        if (coord[i] - 1 >= 0) {
            coord[i]--;
            nid = compute_nid(num_dims, coord, size);
            if (check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]++;
        }
        if (coord[i] + 1 < size[i]) {
            coord[i]++;
            nid = compute_nid(num_dims, coord, size);
            if (check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]--;
        }

        /*Torus neighbors*/
        for (int i = 0; i < num_dims; i++) {
            if (coord[i] == 0) {
                coord[i] = size[i]-1;
                nid = compute_nid(num_dims, coord, size);
                if (check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = 0;
            }

            if (coord[i] == size[i]-1) {
                coord[i] = 0;
                nid = compute_nid(num_dims, coord, size);
                if (check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = size[i]-1;
            }
        }
    }
    return num_neighbors;
}

void printArcs(int num_dims, int *size, double cap) 
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
                        nid = compute_nid(num_dims, coord, size);
                        num_neighbors = compute_neighbors(num_dims, coord, size, neighbors);
                        for (int i = 0; i < num_neighbors; i++) {
                            if (cap < 0.0) {
                                printf("%d %d\n", nid, neighbors[i]);
			    }
                            else {
                                printf("%d %d %8.0f\n", nid, neighbors[i], cap);
			    }
                        }
                    }
                }
            }
        }
    }
}

void getTopology(int *coord, int *size, int *bridge, int *bridgeId)
{
#ifdef __bgq__
    Personality_t personality;

    Kernel_GetPersonality(&personality, sizeof(personality));

    coord[0]  = personality.Network_Config.Acoord;
    coord[1]  = personality.Network_Config.Bcoord;
    coord[2]  = personality.Network_Config.Ccoord;
    coord[3]  = personality.Network_Config.Dcoord;
    coord[4]  = personality.Network_Config.Ecoord;

    size[0]  = personality.Network_Config.Anodes;
    size[1]  = personality.Network_Config.Bnodes;
    size[2]  = personality.Network_Config.Cnodes;
    size[3]  = personality.Network_Config.Dnodes;
    size[4]  = personality.Network_Config.Enodes;

    bridge[0] = personality.Network_Config.cnBridge_A;
    bridge[1] = personality.Network_Config.cnBridge_B;
    bridge[2] = personality.Network_Config.cnBridge_C;
    bridge[3] = personality.Network_Config.cnBridge_D;
    bridge[4] = personality.Network_Config.cnBridge_E;

/*
 * * This is the bridge node, numbered in ABCDE order, E increments first.
 * * It is considered the unique "io node route identifer" because each
 * * bridge node only has one torus link to one io node.
 * */

    *bridgeId = bridge[4] + bridge[3]*size[4] + bridge[2]*size[3]*size[4] + bridge[1]*size[2]*size[3]*size[4] + bridge[0]*size[1]*size[2]*size[3]*size[4];
#endif
}

void generateData(int num_dims, int *size, int num_sources, int factor)
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
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    cap = 2048.0;
    printf("param Capacity :=\n");
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    double demand = 2048.0;
    printf("param: Jobs: Source Destination Demand :=\n");
    for (int i = 0; i < num_sources; i++) {
        for (int j = 0; j < factor; j++) {
            printf("%d %d %d %8.1f\n", i*factor+j, i, num_nodes-num_sources*factor+i*factor+j, demand);
        }
    }
    printf(";");
}

void generateDataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds)
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
    printArcs(num_dims, size, cap);
    for	(int i = 0; i < num_bridges; i++) {
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
    printArcs(num_dims, size, cap);
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
