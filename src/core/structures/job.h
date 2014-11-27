#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>
#include <map>

#include "flow.h"

using namespace std;

struct optiq_job {
    int id;
    int source;
    int dest;
    int demand;
    void *buffer;
    vector<struct optiq_flow> flows;
};

int get_next_dest_from_jobs(vector<struct optiq_job> &jobs, int flow_id, int current_ep);
void optiq_job_read_from_file(const char *file_path, vector<struct optiq_job> *jobs);
void get_flows(int **rGraph, int num_vertices, struct optiq_job &job, int &flow_id);
void optiq_job_print(vector<struct optiq_job> *jobs);
void build_look_up_next_dest_table(vector<struct optiq_job> &jobs, int rank, map<int, int> &next_dest);

#endif
