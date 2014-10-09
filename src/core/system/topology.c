#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "topology.h"

int optiq_compute_nid(int num_dims, int *coord, int *size) 
{
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
#ifdef __bgq__
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
#endif
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

int optiq_compute_neighbors_bgq(int num_dims, int *coord, int *size, int *neighbors) 
{
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

void optiq_read_topology_from_file(char *filePath, struct topology *topo) 
{
    FILE *fp;
    char *line = (char *) malloc(256);;
    size_t len = 0;
    ssize_t read;

    fp = fopen(filePath, "r");
    if (fp == NULL) {
	exit(EXIT_FAILURE);
    }

    /*Skip the first 3 lines*/
    for (int i = 0; i < 3; i++) {
	read = getline(&line, &len, fp);
    }

    getline(&line, &len, fp);
    sscanf(line, "num_dims: %d", &topo->num_dims);

    topo->size = (int *)malloc(sizeof(int) * topo->num_dims);
    getline(&line, &len, fp);
    if (topo->num_dims == 3) {
	sscanf(line, "size: %d x %d x %d", &topo->size[0], &topo->size[1], &topo->size[2]);
    } else if (topo->num_dims == 5) {
	sscanf(line, "size: %d x %d x %d x %d x %d", &topo->size[0], &topo->size[1], &topo->size[2],  &topo->size[3], &topo->size[4]);
    }

    getline(&line, &len, fp);
    sscanf(line, "num_ranks: %d", &topo->num_ranks);

    topo->all_coords = (int **)malloc(sizeof(int *) * topo->num_ranks);
    for (int i = 0; i < topo->num_ranks; i++) {
	topo->all_coords[i] = (int *) malloc(sizeof(int) * topo->num_dims);
    }
    topo->all_nic_ids = (int *) malloc(sizeof(int) * topo->num_ranks);
    
    int coord[5], nid, rank;
    while ((read = getline(&line, &len, fp)) != -1) {
	if (topo->num_dims == 3) {
	    sscanf(line, "Rank: %d nid %d coord[ %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2]);
	}
	if (topo->num_dims == 5) {
            sscanf(line, "Rank: %d nid %d coord[ %d %d %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2], &coord[3], &coord[4]);
        }

	topo->all_nic_ids[rank] = nid;
	for (int i = 0; i < topo->num_dims; i++) {
	    topo->all_coords[rank][i] = coord[i];
	}
    }

    fclose(fp);
}

void optiq_compute_neighbors_cray(int num_dims, int *coord, int **all_coords, int all_ranks, int **neighbors_coords) 
{
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
            optiq_compare_and_replace(coord, neighbor_coords[0], &current_distance[0], all_coords[i], num_dims);
        }

        /*Neighbor in X- direction*/
        if ((coord[0] > all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            optiq_compare_and_replace(coord, neighbors_coords[1], &current_distance[1], all_coords[i], num_dims);
        }

        /*Neighbor in Y+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] < all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            optiq_compare_and_replace(coord, neighbors_coords[2], &current_distance[2], all_coords[i], num_dims);
        }

        /*Neighbor in Y- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] > all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
            optiq_compare_and_replace(coord, neighbors_coords[3], &current_distance[3], all_coords[i], num_dims);
        }

        /*Neighbor in Z+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] < all_coords[i][2])) {
            optiq_compare_and_replace(coord, neighbors_coords[4], &current_distance[4], all_coords[i], num_dims);
        }

        /*Neighbor in Z- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] > all_coords[i][2])) {
            optiq_compare_and_replace(coord, neighbors_coords[5], &current_distance[5], all_coords[i], num_dims);
	}
    }
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
                        num_neighbors = optiq_compute_neighbors_bgq(num_dims, coord, size, neighbors);
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

void optiq_init() 
{
#ifdef _CRAYC
    int rc;
    PMI_BOOL initialized;
    rc = PMI_Initialized(&initialized);
    if (rc!=PMI_SUCCESS) {
	PMI_Abort(rc,"PMI_Initialized failed");
    }

    if (initialized != PMI_TRUE) {
	int spawned;
	rc = PMI_Init(&spawned);
	if (rc!=PMI_SUCCESS) {
	    PMI_Abort(rc,"PMI_Init failed");
        }
    }
#endif
}

void optiq_get_rank(int *rank) 
{
    optiq_init();
#ifdef _CRAYC
    int rc, rank;
    rc = PMI_Get_rank(&rank);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_rank failed");
    }
#endif
}

void optiq_get_num_ranks(int *num_ranks) 
{
    optiq_init();
#ifdef _CRAYC
    int rc, num_ranks;
    rc = PMI_Get_rank(&num_ranks);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_rank failed");
    }
#endif
}

void optiq_get_nic_id(uint16_t *nid) 
{
    int rc, rank;
    optiq_get_rank(&rank);
#ifdef _CRAYC
    /*Get the coordinates of compute nodes*/
    rc = PMI_Get_nid(rank, nid);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_nid failed");
    }
#endif
}

void optiq_get_coord(int *coord)
{
    uint16_t nid;
    optiq_get_nic_id(&nid);

#ifdef _CRAYC
    /*Get the coordinates of compute nodes*/
    pmi_mesh_coord_t xyz;
    PMI_Get_meshcoord(nid, &xyz);

    coord[0] = (int)xyz.mesh_x;
    coord[1] = (int)xyz.mesh_y;
    coord[2] = (int)xyz.mesh_z;
#endif

#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    coord[0] = pers.Network_Config.Acoord;
    coord[1] = pers.Network_Config.Bcoord;
    coord[2] = pers.Network_Config.Ccoord;
    coord[3] = pers.Network_Config.Dcoord;
    coord[4] = pers.Network_Config.Ecoord;
#endif

    optiq_finalize();
}

void optiq_finalize() 
{
#ifdef _CRAYC
    PMI_Finalize();
#endif
}

void optiq_get_size(int *size) 
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

void optiq_get_torus(int *torus) 
{
#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    uint64_t Nflags = pers.Network_Config.NetFlags;
    if (Nflags & ND_ENABLE_TORUS_DIM_A) torus[0] = 1; else torus[0] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_B) torus[1] = 1; else torus[1] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_C) torus[2] = 1; else torus[2] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_D) torus[3] = 1; else torus[3] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_E) torus[4] = 1; else torus[4] = 0;
#endif
}

void optiq_get_bridge(int *bridge_coord, int *brige_id) 
{
#ifdef __bgq__
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    bridge_coord[0] = personality.Network_Config.cnBridge_A;
    bridge_coord[1] = personality.Network_Config.cnBridge_B;
    bridge_coord[2] = personality.Network_Config.cnBridge_C;
    bridge_coord[3] = personality.Network_Config.cnBridge_D;
    bridge_coord[4] = personality.Network_Config.cnBridge_E;

    int size[5];
    optiq_get_size(size);

    *bridge_id = bridge[4] + bridge[3]*size[4] + bridge[2]*size[3]*size[4] + bridge[1]*size[2]*size[3]*size[4] + bridge[0]*size[1]*size[2]*size[3]*size[4];
    
#endif
}

void optiq_get_all_coords(int **all_coords, int num_ranks) 
{
#ifdef __bgq__
    BG_CoordinateMapping_t *coord = (BG_CoordinateMapping_t *) malloc(sizeof(BG_CoordinateMapping_t)*num_ranks);
    
    optiq_map_ranks_to_coords(coord, num_ranks);
    for (int i = 0; i < num_ranks; i++) {
	all_coords[i][0] = coord[i].a;
	all_coords[i][1] = coord[i].b;
	all_coords[i][2] = coord[i].c;
	all_coords[i][3] = coord[i].d;
	all_coords[i][4] = coord[i].e;
    }

    free(coord);
#endif
}

void optiq_get_all_nic_ids(int *all_nic_ids, int num_ranks) 
{

}

void optiq_get_topology(struct topology *topo) 
{
#ifdef __bgq__
    topo->num_dims = 5;
#endif

#ifdef _CRAYC
    topo->num_dims = 3;
#endif

    optiq_get_rank(&topo->rank);
    optiq_get_num_ranks(&topo->num_ranks);
    optiq_get_nic_id(&topo->nic_id);

    topo->coord = (int *)malloc(sizeof(int)*topo->num_dims);
    optiq_get_coord(topo->coord);

    topo->size = (int *)malloc(sizeof(int)*topo->num_dims);
    optiq_get_size(topo->size);

    topo->routing_order = (int *)malloc(sizeof(int)*topo->num_dims);
    optiq_compute_routing_order(topo->num_dims, topo->size, topo->routing_order);

    topo->all_coords = (int **)malloc(sizeof(int *) * topo->num_ranks);
    for (int i = 0; i < topo->num_ranks; i++) {
	topo->all_coords[i] = (int *)malloc(sizeof(int) * topo->num_dims);
    }
    optiq_get_all_coords(topo->all_coords, topo->num_ranks);

    topo->all_nic_ids = (int *) malloc(sizeof(int) * topo->num_ranks);
    optiq_get_all_nic_ids(topo->all_nic_ids, topo->num_ranks);
}
