#ifndef TOPOLOGY_H
#define TOPOLOGY_H

void GetCoordinates(int * coords, int *nid);
void getTopologyInfo(int *coord, int *size);
void getTopologyInfo(int *coord, int *size, int *torus);
void getTopology(int *coord, int *size, int *bridge, int *bridgeId);
void generate_data(int num_dims, int *size);
void generateDataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds);
int check_existing(int num_neighbors, int *neighbors, int nid);
int compute_nid(int num_dims, int *coord, int *size);
int compute_neighbors(int num_dims, int *coord, int *size, int *neighbors);
void printArcs(int num_dims, int *size, double cap);

#endif
