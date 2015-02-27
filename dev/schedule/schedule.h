#ifndef OPTIQ_SCHEDULE_H
#define OPTIQ_SCHEDULE_H

#include "optiq_struct.h"
#include "path.h"
#include "job.h"
#include "pami_transport.h"
#include "comm_mem.h"

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
    int last_path_index;
};

struct optiq_schedule {
    int schedule_id;

    int world_rank;
    int world_size;

    int remaining_jobs;
    int *next_dests;

    int recv_len;
    int expecting_length;

    struct optiq_memregion recv_mr;
    struct optiq_memregion send_mr;

    int *rdispls;
    
    bool isDest;
    bool isSource;

    struct optiq_pami_transport *pami_transport;

    int sent_bytes;
    int *recv_bytes;
    int chunk_size;

    std::vector<struct optiq_job> local_jobs;

    int *all_num_dests;
    int active_immsends;

    std::vector<struct path *> paths;
};

extern "C" struct optiq_schedule *schedule;

void optiq_schedule_init();

void optiq_schedule_finalize();

struct optiq_schedule *optiq_schedule_get();

void optiq_schedule_split_jobs (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size);

void optiq_schedule_split_jobs_multipaths (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size);

void optiq_schedule_add_paths (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths);

void optiq_schedule_print_jobs(struct optiq_schedule &schedule);

void optiq_schedule_mem_destroy(struct optiq_schedule &schedule, struct optiq_pami_transport *pami_transport);

void optiq_schedule_mem_reg (struct optiq_schedule &schedule, struct optiq_comm_mem &comm_mem, struct optiq_pami_transport *pami_transport);

void optiq_schedule_assign_job_demand(std::vector<struct optiq_job> &local_jobs, int nbytes);

void optiq_schedule_set(struct optiq_schedule &schedule, int num_jobs, int world_size);

int optiq_schedule_get_pair(int *sendcounts, std::vector<std::pair<int, std::vector<int> > > &source_dests);

void optiq_schedule_build (void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls);

void optiq_schedule_memory_register(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls,  struct optiq_schedule *schedule);

void optiq_mem_reg(void *buf, int *counts, int *displs, pami_memregion_t &mr);

void optiq_schedule_destroy();
#endif
