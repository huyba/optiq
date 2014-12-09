#include "util.h"
#include "topology.h"

int optiq_compute_nid(int num_dims, int *size, int *coord)
{
    int node_id = coord[num_dims-1];
    int  pre_size = 1;

    for (int i = num_dims-2; i >= 0; i--) {
        for (int j = i+1; j < num_dims; j++) {
            pre_size *= size[j];
        }

        node_id += coord[i]*pre_size;
        pre_size = 1;
    }
    return node_id;
}

int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors)
{
    int num_neighbors = 0;
    int nid = 0;

    for (int i = 0; i < num_dims; i++) {
        if (coord[i] - 1 >= 0) {
            coord[i]--;
            nid = optiq_compute_nid(num_dims, size, coord);
            if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]++;
        }
        if (coord[i] + 1 < size[i]) {
            coord[i]++;
            nid = optiq_compute_nid(num_dims, size, coord);
            if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]--;
        }

        /*Torus neighbors*/
        for (int i = 0; i < num_dims; i++) {
            if (coord[i] == 0) {
                coord[i] = size[i]-1;
                nid = optiq_compute_nid(num_dims, size, coord);
                if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = 0;
            }

            if (coord[i] == size[i]-1) {
                coord[i] = 0;
                nid = optiq_compute_nid(num_dims, size, coord);
                if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = size[i]-1;
            }
        }
    }
    return num_neighbors;
}
