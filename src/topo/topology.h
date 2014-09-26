#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

typedef struct {
    int num_dims;
    int *size;
    int *coord;
    int *all_coords;
    int *routing_order;
} topology;


int optiq_compute_nid(int num_dims, int *coord, int *size);
void optiq_coord_to_nodeId(int num_dims, int *size, int *coord, int *nodeId);
void optiq_move_along_one_dimension(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);
void optiq_reconstruct_path(int num_dims, int *size, int *source, int *dest, int *order, int *torus, int **path);
int optiq_compute_num_hops(int num_dims, int *source, int *dest);
void optiq_compute_routing_order(int num_dims, int *size, int *order);
void optiq_map_ranks_to_coords(BG_CoordinateMapping_t *all_coord, int nranks);
int optiq_check_existing(int num_neighbors, int *neighbors, int nid);
int optiq_compute_neighbors(int num_dims, int *coord, int *size, int *neighbors);
void printArcs(int num_dims, int *size, double cap);
void optiq_get_coordinates(int *coords, int *nid);
void optiq_get_topology_info(int *coord, int *size);
void optiq_get_topology_info(int *coord, int *size, int *torus);
void optiq_get_topology(int *coord, int *size, int *bridge, int *bridgeId);
void optiq_generate_data(int num_dims, int *size);
void optiq_generate_dataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds);

#endif
