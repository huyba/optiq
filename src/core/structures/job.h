#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>

#include "flow.h"

struct optiq_job {
    int id;
    int source;
    int dest;
    int demand;
    void *buffer;
    int num_flows;
    vector<struct optiq_flow> flows;
};

int get_next_dest_from_jobs(vector<struct optiq_job> *jobs, int flow_id, int current_ep);
void read_flow_from_file(char *file_path, vector<struct optiq_job> &jobs);
void get_flows(int **rGraph, int num_vertices, struct optiq_job &job, int &flow_id);
void print_jobs(vector<struct optiq_job> &jobs);

#endif
