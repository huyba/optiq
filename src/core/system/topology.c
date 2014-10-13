#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "topology_interface.h"
#include "topology_bgq.h"
#include "topology_xe6.h"
#include "topoloy_xc30.h"
#include "topology.h"

void optiq_topology_init(struct topology *self, enum machine_type machine)
{
    if (machine == BGQ) {
	self->num_dims = 5;
	self->topo_impl = &topology_bgq;
    } else if (machine == XE6) {
	self->num_dims = 3;
	self->topo_impl = &topology_xe6;
    } else if (machine == XC30) {
	self->num_dims = 3;
	self->topo_impl = &topology_xc30;
    } else {
	/*self->topo_impl = &topology_user_defined;*/
    }
    self->topo_impl->optiq_topology_init();
}

void optiq_topology_get_coord(struct topology *self, int *coord)
{
    self->topo_impl->optiq_topology_get_coord(coord);
}

int optiq_compute_nid(struct topology *self, int num_dims, int *coord, int *size) 
{
    self->topo_impl->optiq_compute_nid(num_dims, coord, size);
}

void optiq_coord_to_nodeId(struct topology *self, int num_dims, int *size, int *coord, int *nodeId)
{
    self->topo_impl->optiq_coord_to_nodeId(num_dims, size, coord, nodeId);
}

void optiq_move_along_one_dimension(struct topology *self, int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path)
{
}

void optiq_reconstruct_path(struct topology *self, int num_dims, int *size, int *source, int *dest, int *order, int *torus, int **path)
{

}

int optiq_compute_num_hops(int num_dims, int *source, int *dest)
{
    int num_hops = 0;
    for (int i = 0; i < num_dims; i++) {
        num_hops += abs(source[i] - dest[i]);
    }
    return num_hops;
}

void optiq_compute_routing_order(struct topology *self, int num_dims, int *size, int *order)
{
}

void optiq_map_ranks_to_coords(BG_CoordinateMapping_t *all_coord, int nranks)
{
}

int optiq_check_existing(int num_neighbors, int *neighbors, int nid) 
{
    for (int i = 0; i < num_neighbors; i++) {
        if (neighbors[i] == nid) {
            return 1;
        }
    }

    return 0;
}

int optiq_compute_neighbors(struct topology *self, int num_dims, int *coord, int *size, int *neighbors) 
{
    
}

void optiq_topology_read_topology_from_file(struct topology *self, char *filePath) 
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
    sscanf(line, "num_dims: %d", &self->num_dims);

    self->size = (int *)malloc(sizeof(int) * self->num_dims);
    getline(&line, &len, fp);
    if (self->num_dims == 3) {
	sscanf(line, "size: %d x %d x %d", &self->size[0], &self->size[1], &self->size[2]);
    } else if (self->num_dims == 5) {
	sscanf(line, "size: %d x %d x %d x %d x %d", &self->size[0], &self->size[1], &self->size[2],  &topo->size[3], &topo->size[4]);
    }

    getline(&line, &len, fp);
    sscanf(line, "num_ranks: %d", &self->num_ranks);

    self->all_coords = (int **)malloc(sizeof(int *) * self->num_ranks);
    for (int i = 0; i < self->num_ranks; i++) {
	self->all_coords[i] = (int *) malloc(sizeof(int) * self->num_dims);
    }
    self->all_nic_ids = (uint16_t *) malloc(sizeof(uint16_t) * self->num_ranks);
    
    int coord[5], nid, rank;
    while ((read = getline(&line, &len, fp)) != -1) {
	if (self->num_dims == 3) {
	    sscanf(line, "Rank: %d nid %d coord[ %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2]);
	}
	if (self->num_dims == 5) {
            sscanf(line, "Rank: %d nid %d coord[ %d %d %d %d %d ]", &rank, &nid, &coord[0], &coord[1], &coord[2], &coord[3], &coord[4]);
        }

	self->all_nic_ids[rank] = nid;
	for (int i = 0; i < self->num_dims; i++) {
	    self->all_coords[rank][i] = coord[i];
	}
    }

    fclose(fp);
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



void optiq_topology_get_rank(int *rank) 
{

}

void optiq_get_num_ranks(int *num_ranks) 
{
}

void optiq_get_nic_id(uint16_t *nid) 
{
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

void optiq_get_all_nic_ids(uint16_t *all_nic_ids, int num_ranks) 
{

}

void optiq_topology_get_topology(struct topology *self, enum machine_type machine) 
{
    self = (struct topology *)malloc(sizeof(struct topology));
    optiq_topology_init(self, machine);
 
    optiq_topology_get_num_ranks(self, &self->num_ranks);

    self->size = (int *)malloc(sizeof(int) * self->num_dims);
    optiq_topology_get_size(self, self->size);

    self->routing_order = (int *)malloc(sizeof(int)*self->num_dims);
    optiq_compute_routing_order(self->num_dims, self->size, self->routing_order);

    self->all_coords = (int **)malloc(sizeof(int *) * self->num_ranks);
    for (int i = 0; i < self->num_ranks; i++) {
	self->all_coords[i] = (int *)malloc(sizeof(int) * self->num_dims);
    }
    optiq_get_all_coords(self->all_coords, self->num_ranks);

    self->all_nic_ids = (uint16_t *) malloc(sizeof(uint16_t) * self->num_ranks);
    optiq_get_all_nic_ids(self->all_nic_ids, self->num_ranks);
}

void optiq_topology_get_node(struct topology *self, struct optiq_node *node, int num_dims)
{
    node = (struct optiq_node *)malloc(sizeof(struct optiq_node));

    optiq_topology_get_rank(self, &node->rank);
    optiq_topology_get_nic_id(self, &node->nic_id);
    node->coord = (int *)malloc(sizeof(int) * num_dims);
    optiq_topology_get_coord(self, node->coord);
}

void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims)
{
    int potential_distance = optiq_compute_num_hops(num_dims, coord, potential_neighbor.node.coord);

    if (potential_distance < current_neighbor->distance) {
	current_neighbor->distance = potential_distance;
	for (int i = 0; i < num_dims; i++) {
	    current_neighbor->node.coord[i] = potential_neighbor.node.coord[i];
	}
    }
}
