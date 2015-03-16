#ifndef OPTIQ_SCHEDULE_H
#define OPTIQ_SCHEDULE_H

#include "util.h"
#include "optiq_struct.h"
#include "path.h"
#include "job.h"
#include "pami_transport.h"
#include "comm_mem.h"

struct optiq_pami_transport; /* Forward declartion of optiq_pami_transport*/

struct optiq_job {
    int job_id;				/* id of the job */
    int source_node_id;			/* source node id of the job */
    int source_rank;			/* source rank id of the job. One node can have many ranks */
    int dest_node_id;			/* destination node id of the job */
    int dest_rank;			/* destination rank id of the job */
    struct optiq_memregion send_mr;	/* memory region of the job. Include pami_memregion and offset */
    int buf_offset;			/* offset of the memory region */
    int buf_length;			/* length the buffer for the job */
    std::vector<struct path *> paths;	/* paths that the job will use to transfer data */
    int last_path_index;		/* the index of the path that was used last time. Assume that multiple paths are available. */
};

enum dequeue_mode {
    DQUEUE_LOCAL_MESSAGE_FIRST,		/* to transfer its own (local) messages first. */
    DQUEUE_FORWARD_MESSAGE_FIRST,	/* to tranfser the forwarding messages first. */
    DQUEUE_ROUND_ROBIN			/* to transfer in the round robin fashion: one local message and then one forwarding message */
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

    std::vector<std::pair<int, std::vector<int> > > notify_list;
    std::vector<std::pair<int, std::vector<int> > > intermediate_notify_list;

    int num_active_paths;

    enum dequeue_mode dmode;
};

extern "C" struct optiq_schedule *schedule;

void optiq_schedule_init();

void optiq_schedule_finalize();

struct optiq_schedule *optiq_schedule_get();

void optiq_schedule_set_dqueue_mode(enum dequeue_mode dmode);

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

void build_notify_lists(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > &notify_list, std::vector<std::pair<int, std::vector<int> > > &intermediate_notify_list, int &num_active_paths, int world_rank);

void optiq_schedule_destroy();

int optiq_schedule_get_chunk_size(int message_size, int sendrank, int recvrank);

void optiq_schedule_map_from_rankpairs_to_idpairs(std::vector<std::pair<int, std::vector<int> > > &source_dest_ranks, std::vector<std::pair<int, std::vector<int> > > &source_dest_ids);

void optiq_schedule_map_from_pathids_to_pathranks (std::vector<struct path *> &path_ids, std::vector<std::pair<int, std::vector<int> > > &source_dest_ranks, std::vector<struct path *> &path_ranks);

void optiq_schedule_print_sourcedests(std::vector<std::pair<int, std::vector<int> > > &source_dests);

#endif
