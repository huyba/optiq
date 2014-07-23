#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int compute_nid(int num_dims, int *coord, int *size) {
    int nid = coord[num_dims-1];
    int  pre_size = 1;

    for(int i = num_dims-2; i >= 0; i--) {
        for(int j = i+1; j < num_dims; j++)
            pre_size *= size[j];

        nid += coord[i]*pre_size;
        pre_size = 1;
    }

    return nid;
}

int check_existing(int num_neighbors, int *neighbors, int nid) {
    for(int i = 0; i < num_neighbors; i++)
        if(neighbors[i] == nid)
            return true;

    return false;
}

int compute_neighbors(int num_dims, int *coord, int *size, int *neighbors) {
    int num_neighbors = 0;
    int nid = 0;

    for(int i = 0; i < num_dims; i++) {
        if(coord[i] - 1 >= 0) {
            coord[i]--;
            nid = compute_nid(num_dims, coord, size);
            neighbors[num_neighbors] = nid;
            num_neighbors++;
            coord[i]++;
        }
        if(coord[i] + 1 < size[i]) {
            coord[i]++;
            nid = compute_nid(num_dims, coord, size);
            neighbors[num_neighbors] = nid;
            num_neighbors++;
            coord[i]--;
        }

        /*Torus neighbors*/
        for(int i = 0; i < num_dims; i++) {
            if(coord[i] == 0) {
                coord[i] = size[i]-1;
                nid = compute_nid(num_dims, coord, size);
                if(!check_existing(num_neighbors, neighbors, nid)) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = 0;
            }

            if(coord[i] == size[i]-1) {
                coord[i] = 0;
                nid = compute_nid(num_dims, coord, size);
                if(!check_existing(num_neighbors, neighbors, nid)) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = size[i]-1;
            }
        }
    }
    return num_neighbors;
}

void printArcs(int num_dims, int *size, double cap) {
    int num_neighbors = 0;
    int neighbors[10];
    int coord[5];
    int nid;

    for(int ad = 0; ad < size[0]; ad++)
    {
	coord[0] = ad;
	for(int bd = 0; bd < size[1]; bd++)
	{
	    coord[1] = bd;
	    for(int cd = 0; cd < size[2]; cd++)
	    {
		coord[2] = cd;
		for(int dd = 0; dd < size[3]; dd++)
		{
		    coord[3] = dd;
		    for(int ed = 0; ed < size[4]; ed++)
		    {
			coord[4] = ed;
			num_neighbors = 0;
			nid = compute_nid(num_dims, coord, size);
			num_neighbors = compute_neighbors(num_dims, coord, size, neighbors);
			for(int i = 0; i < num_neighbors; i++)
			{
			    if(cap < 0.0)
				printf("%d %d\n", nid, neighbors[i]);
			    else
				printf("%d %d %8.0f\n", nid, neighbors[i], cap);
			}
		    }
		}
	    }
	}
    }
}

void main(int argc, char **argv) {
    int num_dims = 5;
    int size[5];

    size[0] = 2;
    size[1] = 2;
    size[2] = 2;
    size[3] = 2;
    size[4] = 2;

    int num_nodes = 1;
    for(int i = 0; i < num_dims; i++)
	num_nodes *= size[i];

    printf("set Nodes :=\n");
    for(int i = 0; i < num_nodes; i++)
	printf("%d\n", i);
    printf(";\n\n");

    printf("set Arcs :=\n");
    double cap = -1.0;
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    cap = 2048.0;
    printf("param Capacity :=\n");
    printArcs(num_dims, size, cap);
    printf(";\n\n");

    printf("param: JobID: Source Destination Demand :=\n");

    printf(";");
}
