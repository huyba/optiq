#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>

#include "path.h"

struct job {
    int job_id;
    int source_id;
    int dest_id;
    int demand;

    std::vector<struct path *> paths;
};

#endif
