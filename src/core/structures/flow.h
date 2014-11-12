#ifndef OPTIQ_FLOW
#define OPTIQ_FLOW

#include <vector>

using namespace std;

struct optiq_arc {
    int ep1;
    int ep2;
};

struct optiq_flow {
    int id;
    int throughput;
    int num_arcs;
    vector<struct optiq_arc> arcs;
};

void read_flow_from_file(char *file_path, vector<struct optiq_job> &jobs);
int get_next_dest_from_flow(const struct optiq_flow &flow, int current_ep);
int get_next_dest_from_jobs(vector<struct optiq_job> &jobs, int flow_id, int current_ep);
void get_flows(int **rGraph, int num_vertices, struct optiq_job &job, int &flow_id);
void print_jobs(vector<struct optiq_job> &jobs);

#endif
