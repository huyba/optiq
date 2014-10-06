#include <stdio.h>
#include <stdlib.h>
#include "topology.h"

int optiq_compute_nid(int num_dims, int *coord, int *size) {
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

void optiq_coord_to_nodeId(int num_dims, int *size, int *coord, int *nodeId)
{
    int temp;
    *nodeId = 0;
    for (int i = 0; i < num_dims; i++) {
        temp = 1;
        for (int j = i+1; j < num_dims; j++) {
            temp *= size[j];
        }
        temp = coord[i]*temp;
        *nodeId += temp;
    }
}

void optiq_move_along_one_dimension(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path)
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

void optiq_reconstruct_path(int num_dims, int *size, int *source, int *dest, int *order, int *torus, int **path)
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

        optiq_move_along_one_dimension(num_dims, size, immediate_node, routing_dimension, num_hops, direction, &path[num_nodes]);

        immediate_node[routing_dimension] = dest[routing_dimension];
        num_nodes += num_hops;
    }
}

int optiq_compute_num_hops(int num_dims, int *source, int *dest)
{
    int num_hops = 0;
    for (int i = 0; i < num_dims; i++) {
        num_hops += abs(source[i] - dest[i]);
    }
    return num_hops;
}

void optiq_compute_routing_order(int num_dims, int *size, int *order)
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

#ifdef __bgq__
void optiq_map_ranks_to_coords(BG_CoordinateMapping_t *all_coord, int nranks)
{
    uint64_t numentries;
    Kernel_RanksToCoords(sizeof(BG_CoordinateMapping_t)*nranks, all_coord, &numentries);
}
#endif

int optiq_check_existing(int num_neighbors, int *neighbors, int nid) 
{
    for (int i = 0; i < num_neighbors; i++) {
        if (neighbors[i] == nid) {
            return 1;
        }
    }

    return 0;
}

int optiq_compute_neighbors(int num_dims, int *coord, int *size, int *neighbors) {
    int num_neighbors = 0;
    int nid = 0;

    for (int i = 0; i < num_dims; i++) {
        if (coord[i] - 1 >= 0) {
            coord[i]--;
            nid = optiq_compute_nid(num_dims, coord, size);
            if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                neighbors[num_neighbors] = nid;
                num_neighbors++;
            }
            coord[i]++;
        }
        if (coord[i] + 1 < size[i]) {
            coord[i]++;
            nid = optiq_compute_nid(num_dims, coord, size);
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
                nid = optiq_compute_nid(num_dims, coord, size);
                if (optiq_check_existing(num_neighbors, neighbors, nid) != 1) {
                    neighbors[num_neighbors] = nid;
                    num_neighbors++;
                }
                coord[i] = 0;
            }

            if (coord[i] == size[i]-1) {
                coord[i] = 0;
                nid = optiq_compute_nid(num_dims, coord, size);
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

void optiq_compute_neighbors(int num_dims, int *coord, int **all_coords, int all_ranks, int **neighbors_coords) {
#ifdef _CRAYC
    int num_dims = 3;

    int current_distance[6];
    int max_dis = 1000000;
    for (int i = 0; i < 6; i++) {
        current_distance[i] = max_dis;
        neighbors_coords[i][0] = -1;
        neighbors_coords[i][1] = -1;
        neighbors_coords[i][2] = -1;
    }

    /*Find the nearest node on a dimension*/
    for (int i = 0; i < all_ranks; i++) {
        /*Neighbor in X+ direction*/
        if ((coord[0] < all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            compare_and_replace(coord, neighbor_coords[0], &current_distance[0], all_coords[i], num_dims);
        }

        /*Neighbor in X- direction*/
        if ((coord[0] > all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            compare_and_replace(coord, neighbors_coords[1], &current_distance[1], all_coords[i], num_dims);
        }

        /*Neighbor in Y+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] < all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            compare_and_replace(coord, neighbors_coords[2], &current_distance[2], all_coords[i], num_dims);
        }

        /*Neighbor in Y- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] > all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            compare_and_replace(coord, neighbors_coords[3], &current_distance[3], all_coords[i], num_dims);
        }

        /*Neighbor in Z+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] < all_coords[i][2])) {
            compare_and_replace(coord, neighbors_coords[4], &current_distance[4], all_coords[i], num_dims);
        }

        /*Neighbor in Z- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] > all_coords[i][2])) {
            compare_and_replace(coord, neighbors_coords[5], &current_distance[5], all_coords[i], num_dims);
	}
    }
#endif
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
                        nid = optiq_compute_nid(num_dims, coord, size);
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

void optiq_get_coordinates(int *coords, int *nid)
{
#ifdef _CRAYC
        int rc, rank;
        PMI_BOOL initialized;
        rc = PMI_Initialized(&initialized);
        if (rc!=PMI_SUCCESS)
                PMI_Abort(rc,"PMI_Initialized failed");

        if (initialized != PMI_TRUE)
        {
                int spawned;
                rc = PMI_Init(&spawned);
                if (rc!=PMI_SUCCESS)
                        PMI_Abort(rc,"PMI_Init failed");
        }

        rc = PMI_Get_rank(&rank);
        if (rc!=PMI_SUCCESS)
                PMI_Abort(rc,"PMI_Get_rank failed");

        /*Get the coordinates of compute nodes*/
        rc = PMI_Get_nid(rank, nid);
        pmi_mesh_coord_t xyz;
        PMI_Get_meshcoord( (uint16_t) *nid, &xyz);

        coords[0] = (int)xyz.mesh_x;
        coords[1] = (int)xyz.mesh_y;
        coords[2] = (int)xyz.mesh_z;

        PMI_Finalize();
#endif
}

void optiq_get_topology_info(int *coord, int *size) 
{
#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    size[0] = pers.Network_Config.Anodes; coord[0] = pers.Network_Config.Acoord;
    size[1] = pers.Network_Config.Bnodes; coord[1] = pers.Network_Config.Bcoord;
    size[2] = pers.Network_Config.Cnodes; coord[2] = pers.Network_Config.Ccoord;
    size[3] = pers.Network_Config.Dnodes; coord[3] = pers.Network_Config.Dcoord;
    size[4] = pers.Network_Config.Enodes; coord[4] = pers.Network_Config.Ecoord;
#endif
}

void optiq_get_topology_info(int *coord, int *size, int *torus) 
{
#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    size[0] = pers.Network_Config.Anodes; coord[0] = pers.Network_Config.Acoord;
    size[1] = pers.Network_Config.Bnodes; coord[1] = pers.Network_Config.Bcoord;
    size[2] = pers.Network_Config.Cnodes; coord[2] = pers.Network_Config.Ccoord;
    size[3] = pers.Network_Config.Dnodes; coord[3] = pers.Network_Config.Dcoord;
    size[4] = pers.Network_Config.Enodes; coord[4] = pers.Network_Config.Ecoord;

    uint64_t Nflags = pers.Network_Config.NetFlags;
    if (Nflags & ND_ENABLE_TORUS_DIM_A) torus[0] = 1; else torus[0] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_B) torus[1] = 1; else torus[1] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_C) torus[2] = 1; else torus[2] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_D) torus[3] = 1; else torus[3] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_E) torus[4] = 1; else torus[4] = 0;
#endif
}

void optiq_get_topology(int *coord, int *size, int *bridge, int *bridgeId)
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
