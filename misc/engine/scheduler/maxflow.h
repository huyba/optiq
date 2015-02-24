#ifndef MFBGQ_H
#define MFBGQ_H

/* C++ program for implementation of Ford Fulkerson algorithm*/
#include "iostream"
#include <limits.h>
#include <string.h>
#include <queue>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct
{
    volatile size_t num_vertices;
    int vertices[1024];
} path_t;

typedef struct
{
    volatile size_t max_flow;
    volatile size_t num_paths;
    path_t paths[0];
} multipath_t;

typedef struct
{
    int routeId;
    int next;
    int sourceId;
    int destId;
    bool isSource;
    bool isProxy;
} route_t;

/* Returns true if there is a path from source 's' to sink 't' in
   residual graph. Also fills parent[] to store the path */
bool bfs(int V, bool *visited, int **rGraph, int s, int t, int parent[]);

/* Returns tne maximum flow from s to t in the given graph */
int fordFulkerson(int V, int **graph, int s, int t, multipath_t *mp);
int fordFulkerson(int V, int **graph, int *sources, int n, int t, multipath_t *mp);
int compute_max_flow(int num_dims, int * sizes, int *sourceId, int nsources, int *destId, int ndest, multipath_t *mp);
void print_path(multipath_t *mp);

#endif
