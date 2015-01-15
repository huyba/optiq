#ifndef OPTIQ_UTIL
#define OPTIQ_UTIL

void trim(char *str);
void rtrim(char *str);
void ltrim(char *str);

bool bfs(int V, bool *visited, int **rGraph, int s, int t, int parent[]);

int optiq_compute_num_hops(int num_dims, int *source, int *dest);
void optiq_compare_and_replace(int *coord, struct optiq_neighbor *current_neighbor, struct optiq_neighbor potential_neighbor, int num_dims);
int optiq_check_existing(int num_elements, int *list, int element);
int optiq_check_existing_neighbor(int num_neighbors, struct optiq_neighbor *neighbors, int nid);

#endif
