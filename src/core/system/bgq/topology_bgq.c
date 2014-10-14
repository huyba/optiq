#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __bgq__

#include "topology_bgq.h"

void optiq_topology_init_bgq(struct topology_info *topo_info)
{

}

void optiq_topology_get_rank_bgq(struct topology_info *topo_info, int *rank)
{
}

void optiq_topology_get_num_ranks_bgq(struct topology_info *topo_info, int *num_ranks)
{

}

void optiq_topology_get_nic_id_bgq(struct topology_info *sefl, uint16_t *nid)
{

}

void optiq_topology_get_coord_bgq(struct topology_info *sefl, int *coord)
{
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    coord[0] = pers.Network_Config.Acoord;
    coord[1] = pers.Network_Config.Bcoord;
    coord[2] = pers.Network_Config.Ccoord;
    coord[3] = pers.Network_Config.Dcoord;
    coord[4] = pers.Network_Config.Ecoord;
}

void optiq_topology_get_all_coords_bgq(struct topology_info *sefl, int **all_coords)
{
    int num_ranks = topo_info->num_ranks;
    BG_CoordinateMapping_t *coord = (BG_CoordinateMapping_t *) malloc(sizeof(BG_CoordinateMapping_t)*num_ranks);

    uint64_t numentries;
    Kernel_RanksToCoords(sizeof(BG_CoordinateMapping_t)*num_ranks, all_coord, &numentries);

    optiq_map_ranks_to_coords(coord, num_ranks);
    for (int i = 0; i < num_ranks; i++) {
	all_coords[i][0] = coord[i].a;
	all_coords[i][1] = coord[i].b;
	all_coords[i][2] = coord[i].c;
	all_coords[i][3] = coord[i].d;
	all_coords[i][4] = coord[i].e;
    }

    free(coord);
}

void optiq_topology_get_all_nic_ids_bgq(struct topology_info *topo_info, uint16_t *all_nic_ids)
{

}

void optiq_topology_get_size_bgq(struct topology_info *topo_info, int *size)
{
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    size[0] = pers.Network_Config.Anodes;
    size[1] = pers.Network_Config.Bnodes;
    size[2] = pers.Network_Config.Cnodes;
    size[3] = pers.Network_Config.Dnodes;
    size[4] = pers.Network_Config.Enodes;
}

void optiq_topology_get_torus_bgq(struct topology_info *topo_info, int *torus)
{
    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    uint64_t Nflags = pers.Network_Config.NetFlags;
    if (Nflags & ND_ENABLE_TORUS_DIM_A) torus[0] = 1; else torus[0] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_B) torus[1] = 1; else torus[1] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_C) torus[2] = 1; else torus[2] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_D) torus[3] = 1; else torus[3] = 0;
    if (Nflags & ND_ENABLE_TORUS_DIM_E) torus[4] = 1; else torus[4] = 0;
}

void optiq_topology_get_bridge_bgq(struct topology_info *topo_info, int *bridge_coord, int *brige_id)
{
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
}

void optiq_topology_get_node_id_bgq(struct topology_info *topo_info, int *coord, int *node_id) 
{
    int num_dims = topo_info->num_dims;

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

void optiq_topology_get_neighbors_bgq(struct topology_info *topo_info, int *coord, optiq_neighbor *neighbors, int *num_neighbors) 
{

    *num_neighbors = 0;
    int nid = 0;
    int *size = topo_info->size;
    int num_dims = topo_info->num_dims;

    for (int i = 0; i < num_dims; i++) {
	if (coord[i] - 1 >= 0) {
	    coord[i]--;
	    optiq_topology_get_node_id_bgq(topo_info, coord, &nid);
	    if (optiq_check_existing(num_neighbors, neighbors.node.rank, nid) != 1) {
		neighbors[num_neighbors].node.rank = nid;
		num_neighbors++;
	    }
	    coord[i]++;
	}
	if (coord[i] + 1 < size[i]) {
	    coord[i]++;
            optiq_topology_get_node_id_bgq(topo_info, coord, &nid);
	    if (optiq_check_existing(num_neighbors, neighbors.node.rank, nid) != 1) {
		neighbors[num_neighbors].node.rank = nid;
		num_neighbors++;
	    }
	    coord[i]--;
	}

	/*Torus neighbors*/
	for (int i = 0; i < num_dims; i++) {
	    if (coord[i] == 0) {
		coord[i] = size[i]-1;
                optiq_topology_get_node_id_bgq(topo_info, coord, &nid);
		if (optiq_check_existing(num_neighbors, neighbors.node.rank, nid) != 1) {
		    neighbors[num_neighbors].node.rank = nid;
		    num_neighbors++;
		}
		coord[i] = 0;
	    }

	    if (coord[i] == size[i]-1) {
		coord[i] = 0;
                optiq_topology_get_node_id_bgq(topo_info, coord, &nid);
		if (optiq_check_existing(num_neighbors, neighbors.node.rank, nid) != 1) {
		    neighbors[num_neighbors].node.rank = nid;
		    num_neighbors++;
		}
		coord[i] = size[i]-1;
	    }
	}
    }
    return num_neighbors;
}

void optiq_topology_get_topology_from_file_bgq(struct topology_info *topo_info, char *filePath) 
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
    sscanf(line, "size: %d x %d x %d x %d x %d", &topo_info->size[0], &topo_info->size[1], &topo_info->size[2],  &topo->size[3], &topo->size[4]);

    getline(&line, &len, fp);
    sscanf(line, "num_ranks: %d", &topo_info->num_ranks);

    topo_info->all_coords = (int **)malloc(sizeof(int *) * topo_info->num_ranks);
    for (int i = 0; i < topo_info->num_ranks; i++) {
	topo_info->all_coords[i] = (int *) malloc(sizeof(int) * topo_info->num_dims);
    }
    topo_info->all_nic_ids = (uint16_t *) malloc(sizeof(uint16_t) * topo_info->num_ranks);

    int coord[5], nid, rank;
    while ((read = getline(&line, &len, fp)) != -1) {
	sscanf(line, "Rank: %d nid %d coord[ %d %d %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2], &coord[3], &coord[4]);

	topo_info->all_nic_ids[rank] = nid;
	for (int i = 0; i < topo_info->num_dims; i++) {
	    topo_info->all_coords[rank][i] = coord[i];
	}
    }

    fclose(fp);
}

void optiq_topology_get_topology_at_runtime_bgq(struct topology_info *topo_info)
{
    topo_info = (struct topology_info *)malloc(sizeof(struct topology));
    topo_info->num_dims = 5;
    optiq_topology_get_num_ranks_bgq(topo_info, &topo_info->num_ranks);

    topo_info->size = (int *)malloc(sizeof(int)*topo_info->num_dims);
    optiq_topology_get_size_bgq(topo_info, topo_info->size);

    topo_info->routing_order = (int *)malloc(sizeof(int)*topo_info->num_dims);
    optiq_topology_compute_routing_order_bgq(topo_info, topo_info->num_dims, topo_info->size, topo_info->routing_order);

    topo_info->all_coords = (int **)malloc(sizeof(int *) * topo_info->num_ranks);
    for (int i = 0; i < topo_info->num_ranks; i++) {
	topo_info->all_coords[i] = (int *)malloc(sizeof(int) * topo_info->num_dims);
    }
    optiq_topology_get_all_coords_bgq(topo_info, topo_info->all_coords);

    topo_info->all_nic_ids = (uint16_t *) malloc(sizeof(uint16_t) * topo_info->num_ranks);
    optiq_topology_get_all_nic_ids_bgq(topo_info, topo_info->all_nic_ids);
}

void optiq_topology_get_node_bgq(struct topology_info *topo_info, struct optiq_node *node)
{
    int num_dims = topo_info->num_dims;
    node = (struct optiq_node *)malloc(sizeof(struct optiq_node));

    optiq_topology_get_rank_bgq(&node->rank);
    optiq_topology_get_nic_id_bgq(&node->nic_id);
    node->coord = (int *)malloc(sizeof(int) * num_dims);
    optiq_topology_get_coord_bgq(node->coord);
}

void optiq_topology_move_along_one_dimension_bgq(struct topology_info *topo_info, int *source, int routing_dimension, int num_hops, int direction, int **path)
{
    int num_dims = topo_info->num_dims;
    int *size = topo_info->size;

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

void optiq_topology_reconstruct_path_bgq(struct topology_info *topo_info, int *source, int *dest, int **path)
{
    int immediate_node[5];

    int *order = topo_info->routing_order;
    int num_dims = topo_info->num_dims;
    int *torus = topo_info->torus;

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

void optiq_topology_compute_routing_order_bgq(struct topology_info *topo_info, int *order)
{
    int num_dims = topo_info->num_dims;
    int *size = topo_info->size;

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

void optiq_topology_print_arcs_bgq(struct topology_info *topo_info, double cap) 
{
    int num_neighbors = 0;
    struct optiq_neighbor *neighbors = (struct optiq_neighbor *)malloc(sizeof(struct optiq_neighbor) * 10);
    int coord[5];
    int nid;

    int *size = topo_info->size;
    int num_dims = topo_info->num_dims;

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
                        optiq_topology_get_node_id_bgq(topo_info, coord, &nid);
			optiq_topology_get_neighbors_bgq(topo_info, coord, neighbors, &num_neighbors);
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

void optiq_topology_finalize_bgq(struct topology_info *sefl)
{
}

struct topology_interface topology_bgq =
{
    .machine = BGQ,
    .optiq_topology_init = optiq_topology_init_bgq,
    .optiq_topology_get_rank = optiq_topology_get_rank_bgq,
    .optiq_topology_get_num_ranks = optiq_topology_get_num_ranks_bgq,
    .optiq_topology_get_nic_id = optiq_topology_get_nic_id_bgq,
    .optiq_topology_get_coord = optiq_topology_get_coord_bgq,
    .optiq_topology_get_all_coords = optiq_topology_get_all_coords_bgq,
    .optiq_topology_get_all_nic_ids = optiq_topology_get_all_nic_ids_bgq,
    .optiq_topology_get_size = optiq_topology_get_size_bgq,
    .optiq_topology_get_torus = optiq_topology_get_torus_bgq,
    .optiq_topology_get_bridge = optiq_topology_get_bridge_bgq,
    .optiq_topology_get_node_id = optiq_topology_get_node_id_bgq,
    .optiq_topology_compute_neighbors = optiq_topology_compute_neighbors_bgq,
    .optiq_topology_get_topology_from_file = optiq_topology_get_topology_from_file_bgq,
    .optiq_topology_get_topology_at_runtime = optiq_topology_get_topology_at_runtimebgq,
    .optiq_topology_get_node = optiq_topology_get_node_bgq,
    .optiq_topology_finalize = optiq_topology_finalize_bgq
};

#endif
