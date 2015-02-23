#include "../utils/util.h"
#include "topology_bgq.h"

void optiq_topology_get_node_id_from_coord_bgq(int num_dims, int *size, int *coord, int *node_id)
{
    *node_id = coord[num_dims-1];
    int  pre_size = 1;

    for (int i = num_dims-2; i >= 0; i--) {
        for (int j = i+1; j < num_dims; j++) {
            pre_size *= size[j];
        }

        *node_id += coord[i]*pre_size;
        pre_size = 1;
    }
}

void optiq_topology_get_neighbor_ids_bgq(int num_dims, int *size, int *coord, int *neighbors, int *num_neighbors)
{
    *num_neighbors = 0;
    int nid = 0;

    for (int i = 0; i < num_dims; i++) {
        if (coord[i] - 1 >= 0) {
            coord[i]--;
            optiq_topology_get_node_id_from_coord_bgq(num_dims, size, coord, &nid);
            if (optiq_check_existing(*num_neighbors, neighbors, nid) != 1) {
                neighbors[*num_neighbors] = nid;
                (*num_neighbors)++;
            }
            coord[i]++;
        }
        if (coord[i] + 1 < size[i]) {
            coord[i]++;
            optiq_topology_get_node_id_from_coord_bgq(num_dims, size, coord, &nid);
            if (optiq_check_existing(*num_neighbors, neighbors, nid) != 1) {
                neighbors[*num_neighbors] = nid;
                (*num_neighbors)++;
            }
            coord[i]--;
        }

        /*Torus neighbors*/
        for (int i = 0; i < num_dims; i++) {
            if (coord[i] == 0) {
                coord[i] = size[i]-1;
                optiq_topology_get_node_id_from_coord_bgq(num_dims, size, coord, &nid);
                if (optiq_check_existing(*num_neighbors, neighbors, nid) != 1) {
                    neighbors[*num_neighbors] = nid;
                    (*num_neighbors)++;
                }
                coord[i] = 0;
            }

            if (coord[i] == size[i]-1) {
                coord[i] = 0;
                optiq_topology_get_node_id_from_coord_bgq(num_dims, size, coord, &nid);
                if (optiq_check_existing(*num_neighbors, neighbors, nid) != 1) {
                    neighbors[*num_neighbors] = nid;
                    (*num_neighbors)++;
                }
                coord[i] = size[i]-1;
            }
        }
    }
}
