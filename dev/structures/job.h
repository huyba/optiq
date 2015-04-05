#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>

#include "path.h"

struct optiq_memregion;

struct job {
    char *name;
    int job_id;
    int source_id;
    int dest_id;
    int source_rank;
    int dest_rank;
    int demand;

    std::vector<struct path *> paths;
};

bool optiq_jobs_read_from_file (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath);

void optiq_job_write_to_file (std::vector<struct job> &jobs, char *filepath);

void optiq_job_print(std::vector<struct job> &jobs, int rank);

void optiq_job_print_jobs (std::vector<struct job> &jobs);

void optiq_job_map_jobs_to_source_dests (std::vector<struct job> &jobs, std::vector<std::pair<int, std::vector<int> > > &source_dests);

#endif
