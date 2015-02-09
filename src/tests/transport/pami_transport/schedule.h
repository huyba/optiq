#ifndef OPTIQ_SCHEDULE
#define OPTIQ_SCHEDULE

#include "path.h"

struct optiq_pami_transport;
struct optiq_memregion;

struct optiq_schedule {
    int schedule_id;

    int world_rank;
    int world_size;

    int remaining_jobs;
    int *next_dests;

    int expecting_length;

    char *send_buf;
    int *sendcounts;
    int *sdispls;

    char *recv_buf;
    int *recvcounts;
    int *rdispls;

    bool isDest;
    bool isSource;

    struct optiq_memregion recv_mr;
    struct optiq_memregion send_mr;

    struct optiq_pami_transport *pami_transport;
};

void optiq_schedule_init(struct optiq_schedule &schedule);

void optiq_schedule_create (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths);

#endif
