#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>

#include "virtual_lane.h"
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

void add_job_to_virtual_lanes(const optiq_job &job, vector<struct optiq_virtual_lane> &virtual_lanes);

#endif
