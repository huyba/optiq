/* 
 * Contains code for topology related functions.
 *
 * */

#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

#include<vector>

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#endif

#include "path.h"

struct optiq_topology {
    int num_dims;
    int num_nodes;
    int num_edges;

    int size[5];
    int torus[5];
    int order[5];

    int bridge[5];
    int bridge_id;

    int coord[5];
    int **all_coords;
    std::vector<int> *neighbors;

    int world_rank;
    int world_size;
    int num_ranks_per_node;
    bool initialized;
    bool finalized;
};

extern "C" struct optiq_topology *topo;

/* Init the topology */
void optiq_topology_init();

/* Init with parameters if not running on a given topology*/
void optiq_topology_init_with_params(int num_dims, int *size, struct optiq_topology *topo);

/* Get the topology after init */
struct optiq_topology* optiq_topology_get();

/* Print the basic topology's information */
void optiq_topology_print(struct optiq_topology *topo);

/* Print the basic information */
void optiq_topology_print_basic(struct optiq_topology *topo);

/* Get the size of the partition such as 2 x 2 x 4 x 4 x2 */
void optiq_topology_get_size_bgq(int *size);

/* Get the node id*/
int optiq_topology_get_node_id(int world_rank);

/* Get a random rank to transfer as the next destination if multiple ranks are available in the next destination node - obsolete method */
int optiq_topology_get_random_rank (int node_id);

/* Calculate the number of hops between 2 nodes */
int optiq_topology_get_hop_distance_2nodes(int node1, int node2);

/* Calculate the number of hops based on ranks*/
int optiq_topology_get_hop_distance(int rank1, int rank2);

/* Compute the maximum distance between 2 sets of soures and destinations*/
int optiq_topology_max_distance_2sets(std::vector<std::pair<int, std::vector<int> > > &source_dests);

/* compute the max distance with torus included */
int optiq_topology_max_distance_2sets_with_torus (std::vector<std::pair<int, std::vector<int> > > &source_dests);

/* Get coordinates of the current node */
int optiq_topology_get_coord(int *coord);

/* Get the bridge node on bgq*/
void optiq_topology_get_bridge_bgq(struct optiq_topology *topo);

/* Compute the node id based on size and coordinate */
int optiq_topology_compute_node_id(int num_dims, int *size, int *coord);

/* Compute the neighbors of a node based on its coordinates and partition size */
int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors);

/* Get all nodes neighbors */
std::vector<int> * optiq_topology_get_all_nodes_neighbors(int num_dims, int *size);

/* Get all coordinates of all nodes */
int** optiq_topology_get_all_coords (int num_dims, int *size);

/* Get torus of the given partition */
void optiq_topology_get_torus(int *torus);

/* Calculate routing order of BGQ */
void optiq_topology_compute_routing_order_bgq(int num_dims, int *size, int *order);

/* Compute how data is moved along one dimension */
void optiq_topology_move_along_one_dimension_bgq(int num_dims, int *size, int *source, int routing_dimension, int num_hops, int direction, int **path);

/* Reconstruct paths for data movement by MPI for BGQ */
void optiq_topology_reconstruct_path_bgq(int num_dims, int *size, int *torus, int *order, int *source, int *dest, int **path);

/* Remove intermediate nodes that comply with default routing algorithms */
void optiq_topology_reduce_intermediate_nodes (std::vector<struct path *> paths, std::vector<struct path *> &reduced_paths);

/* Get routing dimension order */
void optiq_topology_route_along_dimension (int *scoord, int routing_dimension, int destcoord, struct path &p);

/* Reconstruct MPI path for 1 pair of source and destination */
void optiq_topolog_reconstruct_path (int source, int dest, struct path &p);

/* Print all arcs */
void optiq_topology_print_all_arcs(int num_dims, int *size, double cap);

/* Free memory and resources */
void optiq_topology_finalize();

/* Write a graph of the given partition. Used for Yen's or paths listing algorithms */
void optiq_topology_write_graph(struct optiq_topology *topo, int cost, char *filePath);

#endif
