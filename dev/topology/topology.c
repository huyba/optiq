#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "util.h"
#include "topology.h"

void optiq_topology_get_size_bgq(int *size)
{
#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    size[0] = pers.Network_Config.Anodes;
    size[1] = pers.Network_Config.Bnodes;
    size[2] = pers.Network_Config.Cnodes;
    size[3] = pers.Network_Config.Dnodes;
    size[4] = pers.Network_Config.Enodes;
#endif
}

void optiq_topology_print_graph(struct topology *topo, int cost)
{
    for (int i = 0; i < topo->num_nodes; i++)
    {
	for (int j = 0; j < topo->neighbors[i].size(); j++) {
	    printf("%d %d %d\n", i, topo->neighbors[i][j], cost);
	}
    }
}

void optiq_topology_init(int num_dims, int *size, struct topology *topo)
{
    topo->num_dims = num_dims;
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
        num_nodes *= size[i];
	topo->size[i] = size[i];
    }
    topo->num_nodes = num_nodes;
   
    topo->neighbors = optiq_topology_get_all_nodes_neighbors(num_dims, size);

    topo->num_edges = 0;
    for (int i = 0; i < num_nodes; i++) {
	topo->num_edges += topo->neighbors[i].size();
    }
}

int optiq_topology_compute_node_id(int num_dims, int *size, int *coord)
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

    for (int i = num_dims - 1; i >= 0; i--) {
        if (coord[i] - 1 >= 0) {
            coord[i]--;
            nid = optiq_topology_compute_node_id(num_dims, size, coord);
            if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]++;
        }
        if (coord[i] + 1 < size[i]) {
            coord[i]++;
            nid = optiq_topology_compute_node_id(num_dims, size, coord);
            if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]--;
        }

        /*Torus neighbors*/
        for (int i = num_dims - 1; i >= 0; i--) {
            if (coord[i] == 0) {
                coord[i] = size[i]-1;
                nid = optiq_topology_compute_node_id(num_dims, size, coord);
                if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = 0;
            }

            if (coord[i] == size[i]-1) {
                coord[i] = 0;
                nid = optiq_topology_compute_node_id(num_dims, size, coord);
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

std::vector<int> * optiq_topology_get_all_nodes_neighbors(int num_dims, int *size)
{
    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
        num_nodes *= size[i];
    }

    std::vector<int> *all_nodes_neighbors = (std::vector<int> *) calloc(1, sizeof(std::vector<int>) * num_nodes);

    int coord[5], nid = 0, neighbors[10], num_neighbors = 0;

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

                        nid = optiq_topology_compute_node_id(num_dims, size, coord);
                        num_neighbors = optiq_compute_neighbors(num_dims, size, coord, neighbors);

                        for(int i = 0; i < num_neighbors; i++) {
                            all_nodes_neighbors[nid].push_back(neighbors[i]);
                        }
                    }
                }
            }
        }
    }

    return all_nodes_neighbors;
}

void optiq_topology_move_along_one_dimension_bgq(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path)
{
    int dimension_value = source[routing_dimension];
    for (int i = 0; i < num_hops; i++) {
        for (int d = 0; d < num_dims; d++) {
            if (d != routing_dimension) {
                path[i][d] = source[d];
            } else {
                dimension_value = (dimension_value + direction + size[d]) % size[d];
                path[i][d] = dimension_value;
            }
        }
    }
}

void optiq_topology_reconstruct_path_bgq(int num_dims, int *size, int *torus, int *order, int *source, int *dest, int **path)
{
    int immediate_node[5];

    /*Add source node*/
    for (int i = 0; i < num_dims; i++) {
        path[0][i] = source[i];
        immediate_node[i] = source[i];
    }

    /*Add intermedidate nodes*/
    int num_nodes = 1, direction = 0;
    int routing_dimension, num_hops;

    for (int i = 0; i < num_dims; i++) {
        routing_dimension = order[i];
        num_hops = abs(dest[routing_dimension]-source[routing_dimension]);
        if (num_hops == 0) {
            continue;
        }
        direction = (dest[routing_dimension] - source[routing_dimension])/num_hops;

        /*If there is torus link, the direction may change*/
        if (torus[routing_dimension] == 1) {
            if (num_hops > size[routing_dimension]/2) {
                direction *= -1;
            }
        }

        optiq_topology_move_along_one_dimension_bgq(num_dims, size, immediate_node, routing_dimension, num_hops, direction, &path[num_nodes]);

        immediate_node[routing_dimension] = dest[routing_dimension];
        num_nodes += num_hops;
    }
}

void optiq_topology_compute_routing_order_bgq(int num_dims, int *size, int *order)
{
    int num_nodes = 1, dims[5];
    for (int i = 0; i < num_dims; i++) {
        num_nodes *= size[i];
        dims[i] = size[i];
    }

    int longest_dimension, length;
    for (int i = 0; i < num_dims; i++) {
        longest_dimension = i;
        length = dims[i];
        for (int j = 0; j < num_dims; j++) {
            if(dims[j] > length) {
                longest_dimension = j;
                length = dims[j];
            }
        }

        if ((longest_dimension == 0) && (dims[0] == dims[1]) && (num_nodes == 128 || num_nodes == 256)) {
            longest_dimension = 1;
        }

        order[i] = longest_dimension;
        dims[longest_dimension] = -1;
    }
}

void optiq_topology_print_all_arcs(int num_dims, int *size, double cap)
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
                        nid = optiq_topology_compute_node_id(num_dims, coord, size);
                        num_neighbors = optiq_compute_neighbors(num_dims, coord, size, neighbors);
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