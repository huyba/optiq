#ifndef OPTIQ_SCHEDULE_H
#define OPTIQ_SCHEDULE_H

#include "optiq_struct.h"
#include "path.h"
#include "job.h"
#include "pami_transport.h"

struct optiq_pami_transport;

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

struct optiq_schedule {
    int schedule_id;

    int world_rank;
    int world_size;

    int remaining_jobs;
    int *next_dests;

    int expecting_length;

    struct optiq_memregion recv_mr;
    struct optiq_memregion send_mr;

    char *send_buf;
    int *sendcounts;
    int *sdispls;

    char *recv_buf;
    int *recvcounts;
    int *rdispls;

    bool isDest;
    bool isSource;

    struct optiq_memregion *recv_memregions;
    struct optiq_memregion *send_memregions;

    struct optiq_pami_transport *pami_transport;

    int sent_bytes;
    int *recv_bytes;
    int chunk_size;

    std::vector<struct optiq_job> local_jobs;
};

void optiq_schedule_init(struct optiq_schedule &schedule);

void optiq_schedule_split_jobs (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size);

void optiq_schedule_create (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths);

#endif
