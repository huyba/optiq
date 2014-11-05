#ifndef OPTIQ_FLOW
#define OPTIQ_FLOW

#include <queue>

using namespace std;

struct optiq_arc {
    int ep1;
    int ep2;
};

struct optiq_flow {
    int id;
    int throughput;
    int num_arcs;
    struct optiq_arc *arcs;
};

struct optiq_job {
    int id;
    int source;
    int dest;
    int demand;
    int num_flows;
    queue<struct optiq_flow *> flows;
};

void read_flows_from_file(char *filePath);

void get_flows(int **rGraph, int num_vertices, struct optiq_job *job, int *flow_id);
void read_flow_from_file(char *file_path, struct optiq_job *job, int num_jobs);

#endif
