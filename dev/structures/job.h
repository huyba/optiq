#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>

#include "path.h"

struct optiq_memregion;

struct job {
    int job_id;
    int source_id;
    int dest_id;
    int demand;

    std::vector<struct path *> paths;
};

void optiq_job_print(std::vector<struct job> &jobs, int rank);

void optiq_job_map_jobs_to_source_dests (std::vector<struct job> &jobs, std::vector<std::pair<int, std::vector<int> > > &source_dests);

bool optiq_job_read_flow_value_from_file (char *filePath, std::vector<struct job> &jobs);

#endif
