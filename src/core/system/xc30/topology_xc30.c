#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "topology_xc30.h"

void optiq_topology_init_xc30(struct topology_info *topo_info)
{
#ifdef __CRAYXC
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

void optiq_topology_get_rank_xc30(struct topology_info *topo_info, int *rank)
{
#ifdef __CRAYXC
    optiq_topology_init_xc30(topo_info);
    int rc = PMI_Get_rank(rank);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_rank failed");
    }
#endif
}

void optiq_topology_get_num_ranks_xc30(struct topology_info *topo_info, int *num_ranks)
{
#ifdef __CRAYXC
    optiq_topology_init_xc30(topo_info);
    int rc = PMI_Get_size(num_ranks);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_rank failed");
    }
#endif
}

void optiq_topology_get_node_id_from_coord_xc30(struct topology_info *topo_info, int *coord, int *nid)
{
}

void optiq_topology_get_node_id_xc30(struct topology_info *topo_info, int *nid)
{
#ifdef __CRAYXC
    int rc, rank;
    optiq_topology_get_rank_xc30(topo_info, &rank);

    /*Get the coordinates of compute nodes*/
    rc = PMI_Get_nid(rank, nid);
    if (rc!=PMI_SUCCESS) {
        PMI_Abort(rc,"PMI_Get_nid failed");
    }
#endif
}

void optiq_topology_get_coord_xc30(struct topology_info *topo_info, int *coord)
{
#ifdef __CRAYXC
    int nid;
    optiq_topology_get_node_id_xc30(topo_info, &nid);

    /*Get the coordinates of compute nodes*/
    pmi_mesh_coord_t xyz;
    PMI_Get_meshcoord(nid, &xyz);

    coord[0] = (int)xyz.mesh_x;
    coord[1] = (int)xyz.mesh_y;
    coord[2] = (int)xyz.mesh_z;
#endif
}

void optiq_topology_get_physical_location_xc30(struct topology_info *topo_info, int *coord, physical_location *pl)
{
    pl->group_id = coord[0];
    pl->cabinet_id = coord[1]/3;
    pl->chasis_id = coord[1]%3;
    pl->blade_id = coord[2];
}

void optiq_topology_get_all_coords_xc30(struct topology_info *topo_info, int **all_coords)
{

}

void optiq_topology_get_all_node_ids_xc30(struct topology_info *topo_info, int *all_node_ids)
{

}

void optiq_topology_get_size_xc30(struct topology_info *topo_info, int *size)
{
}

void optiq_topology_get_torus_xc30(struct topology_info *topo_info, int *torus)
{
}

void optiq_topology_get_bridge_xc30(struct topology_info *topo_info, int *bridge_coord, int *bridge_id)
{

}

void optiq_topology_get_topology_from_file_xc30(struct topology_info *topo_info, char *filePath) 
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
    sscanf(line, "num_dims: %d", &topo_info->num_dims);

    topo_info->size = (int *)malloc(sizeof(int) * topo_info->num_dims);
    getline(&line, &len, fp);
    sscanf(line, "size: %d x %d x %d", &topo_info->size[0], &topo_info->size[1], &topo_info->size[2]);
 
    getline(&line, &len, fp);
    sscanf(line, "num_ranks: %d", &topo_info->num_ranks);

    topo_info->all_coords = (int **)malloc(sizeof(int *) * topo_info->num_ranks);
    for (int i = 0; i < topo_info->num_ranks; i++) {
	topo_info->all_coords[i] = (int *) malloc(sizeof(int) * topo_info->num_dims);
    }
    topo_info->all_node_ids = (int *) malloc(sizeof(int) * topo_info->num_ranks);
    
    int coord[5], nid, rank;
    while ((read = getline(&line, &len, fp)) != -1) {
	sscanf(line, "Rank: %d nid %d coord[ %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2]);

	topo_info->all_node_ids[rank] = nid;
	for (int i = 0; i < topo_info->num_dims; i++) {
	    topo_info->all_coords[rank][i] = coord[i];
	}
    }

    fclose(fp);
}

void optiq_topology_get_neighbors_xc30(struct topology_info *topo_info, int *coord, struct optiq_neighbor *neighbors, int *num_neighbors) 
{
    int num_dims = topo_info->num_dims;
    int **all_coords = topo_info->all_coords;
    int all_ranks = topo_info->num_ranks;

    int max_dis = 1000000;
    for (int i = 0; i < num_dims * 2; i++) {
        neighbors[i].node.coord[0] = -1;
	neighbors[i].node.coord[1] = -1;
	neighbors[i].node.coord[2] = -1;
	neighbors[i].distance = max_dis;
	neighbors[i].link_capacity = 0;
	neighbors[i].direction = MIXED;
    }

    struct optiq_neighbor potential_neighbor;

    /*Find the nearest node on a dimension*/
    for (int i = 0; i < all_ranks; i++) {
	potential_neighbor.node.rank = i;
	for (int j = 0; j < num_dims; j++) {
	    potential_neighbor.node.coord[i] = all_coords[i][j];
	}

        /*Neighbor in X+ direction*/
        if ((coord[0] < all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
	    potential_neighbor.direction = X_P;
            optiq_compare_and_replace(coord, &neighbors[0], potential_neighbor, num_dims);
        }

        /*Neighbor in X- direction*/
        if ((coord[0] > all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
	    potential_neighbor.direction = X_M;
            optiq_compare_and_replace(coord, &neighbors[1], potential_neighbor, num_dims);
        }

        /*Neighbor in Y+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] < all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
	    potential_neighbor.direction = Y_P;
            optiq_compare_and_replace(coord, &neighbors[2], potential_neighbor, num_dims);
        }

        /*Neighbor in Y- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] > all_coords[i][1]) && (coord[2] == all_coords[i][2])) {
	    potential_neighbor.direction = Y_M;
            optiq_compare_and_replace(coord, &neighbors[3], potential_neighbor, num_dims);
        }

        /*Neighbor in Z+ direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] < all_coords[i][2])) {
	    potential_neighbor.direction = Z_P;
            optiq_compare_and_replace(coord, &neighbors[4], potential_neighbor, num_dims);
        }

        /*Neighbor in Z- direction*/
        if ((coord[0] == all_coords[i][0]) && (coord[1] == all_coords[i][1]) && (coord[2] > all_coords[i][2])) {
	    potential_neighbor.direction = Z_M;
            optiq_compare_and_replace(coord, &neighbors[5], potential_neighbor, num_dims);
	}
    }
}

void optiq_topology_get_topology_at_runtime_xc30(struct topology_info *topo_info) 
{
    optiq_topology_get_num_ranks_xc30(topo_info, &topo_info->num_ranks);

    topo_info->all_coords = (int **)malloc(sizeof(int *) * topo_info->num_ranks);
    for (int i = 0; i < topo_info->num_ranks; i++) {
	topo_info->all_coords[i] = (int *)malloc(sizeof(int) * topo_info->num_dims);
    }
    optiq_topology_get_all_coords_xc30(topo_info, topo_info->all_coords);

    topo_info->all_node_ids = (int *) malloc(sizeof(int) * topo_info->num_ranks);
    optiq_topology_get_all_node_ids_xc30(topo_info, topo_info->all_node_ids);
}

void optiq_topology_get_node_xc30(struct topology_info *topo_info, struct optiq_node *node)
{
    int num_dims = topo_info->num_dims;
    node = (struct optiq_node *)malloc(sizeof(struct optiq_node));

    optiq_topology_get_rank_xc30(topo_info, &node->rank);
    optiq_topology_get_node_id_xc30(topo_info, &node->node_id);
    node->coord = (int *)malloc(sizeof(int) * num_dims);
    optiq_topology_get_coord_xc30(topo_info, node->coord);
}

void optiq_topology_finalize_xc30(struct topology_info *topo_info)
{
}

struct topology_interface topology_xc30 =
{
#ifdef __CRAYXC
    .machine = XC30,
    .optiq_topology_init = optiq_topology_init_xc30,
    .optiq_topology_get_rank = optiq_topology_get_rank_xc30,
    .optiq_topology_get_num_ranks = optiq_topology_get_num_ranks_xc30,
    .optiq_topology_get_node_id = optiq_topology_get_node_id_xc30,
    .optiq_topology_get_node_id_from_coord = optiq_topology_get_node_id_from_coord_xc30,
    .optiq_topology_get_coord = optiq_topology_get_coord_xc30,
    .optiq_topology_get_physical_location = optiq_topology_get_physical_location_xc30,
    .optiq_topology_get_all_coords = optiq_topology_get_all_coords_xc30,
    .optiq_topology_get_all_node_ids = optiq_topology_get_all_node_ids_xc30,
    .optiq_topology_get_size = optiq_topology_get_size_xc30,
    .optiq_topology_get_torus = optiq_topology_get_torus_xc30,
    .optiq_topology_get_bridge = optiq_topology_get_bridge_xc30,
    .optiq_topology_get_neighbors = optiq_topology_get_neighbors_xc30,
    .optiq_topology_get_topology_from_file = optiq_topology_get_topology_from_file_xc30,
    .optiq_topology_get_topology_at_runtime = optiq_topology_get_topology_at_runtime_xc30,
    .optiq_topology_get_node = optiq_topology_get_node_xc30,
    .optiq_topology_finalize = optiq_topology_finalize_xc30
#endif
};
