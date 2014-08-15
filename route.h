#ifndef ROUTE_H
#define ROUTE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int u;
    int v;
    float capacity;
    float load;
} arc_t;

typedef struct {
    int routeId;
    int source;
    int dest;
    arc_t *arcs;
    int num_arcs;
} route_t;

typedef struct {
    int jobId;
    int source;
    int dest;
    float demand;
    route_t *routes;
    int num_routes;
} job_t;

void readRoutes(char *filePath);

#endif
