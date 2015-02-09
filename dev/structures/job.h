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

struct optiq_job {
    int job_id;
    int source_node_id;
    int source_rank;
    int dest_node_id;
    int dest_rank;
    struct optiq_memregion send_mr;
    int buf_offset;
    int buf_length;
    std::vector<struct path *> paths;
};

void optiq_job_print(std::vector<struct job> &jobs, int rank);

#endif
