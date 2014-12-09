#ifndef OPTIQ_TOPOLOGY_BGQ_H
#define OPTIQ_TOPOLOGY_BGQ_H

int optiq_compute_nid(int num_dims, int *size, int *coord);

int optiq_compute_neighbors(int num_dims, int *size, int *coord, int *neighbors);

#endif
