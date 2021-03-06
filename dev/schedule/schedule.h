/*
 * Schedule contains code for scheduling data movement, split buffer of data into queues. Each message contains necessary data for data movement from original source original dest and next destination. It also create routing table to route data from a node to the next node.
 *
 * */
#ifndef OPTIQ_SCHEDULE_H
#define OPTIQ_SCHEDULE_H

#include "util.h"
#include "optiq_struct.h"
#include "path.h"
#include "job.h"
#include "pami_transport.h"
#include "comm_mem.h"

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)

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
    int current_path_index;		/* the index of the path that is currently used time. Assume that multiple paths are available. */
};

enum dequeue_mode {
    DQUEUE_LOCAL_MESSAGE_FIRST,		/* to transfer its own (local) messages first. */
    DQUEUE_FORWARD_MESSAGE_FIRST,	/* to tranfser the forwarding messages first. */
    DQUEUE_ROUND_ROBIN			/* to transfer in the round robin fashion: one local message and then one forwarding message */
};

/* Schedule structure */
struct optiq_schedule {
    int schedule_id;

    int world_rank;
    int world_size;

    int remaining_jobs;
    int *next_dests;

    int send_len;
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

    int active_immsends;

    std::vector<std::pair<int, std::vector<int> > > notify_list;
    std::vector<std::pair<int, std::vector<int> > > intermediate_notify_list;

    int num_active_paths;

    enum dequeue_mode dmode;

    bool auto_chunksize;
    int maxnumpaths;

    std::vector<struct path *> paths;
    std::vector<struct job> jobs;
    int test_id;
};

/* 
 * Global single pointer variable for schedule 
 * */
extern "C" struct optiq_schedule *schedule;

/*
 * Initialize schedule, allocate memories and necessary resources.
 * */
void optiq_schedule_init();

/* Get the currently used schedule by return the ponter to schedule structure */
struct optiq_schedule *optiq_schedule_get();

/*
 * Reassign path ids for each job to make sure that the path ids are unique at the runtime. Path ids are used to route data.
 * */
void optiq_schedule_assign_path_ids_to_jobs (std::vector<struct path *> &path_ids, std::vector<struct job> &jobs, std::vector<struct path *> &path_ranks, int ranks_per_node);

/* Build a list of nodes that are notified when a job is done so everyone can finish and exit waiting for transfer complete */
void build_notify_lists(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > &notify_list, std::vector<std::pair<int, std::vector<int> > > &intermediate_notify_list, int &num_active_paths, int world_rank);

/*Register a memory region */
void optiq_mem_reg(void *buf, int *counts, int *displs, pami_memregion_t &mr);

/* Schedule's memory register for all necessary buffers */
void optiq_schedule_memory_register(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls,  struct optiq_schedule *schedule);

/* Compute the assigned length of memories, offsets for each paths */
void optiq_schedule_compute_assinged_len_for_path (std::vector<struct optiq_job> &jobs);

/* Split the data of jobs into multiple paths */
void optiq_schedule_split_jobs_multipaths (struct optiq_pami_transport *pami_transport, std::vector<struct optiq_job> &jobs, int chunk_size);

/* Compute the path hopbyte for data movement a long paths */
void optiq_schedule_compute_path_hopbyte_copy_stat(std::vector<struct optiq_job> &jobs);

/* Destroy the memory allocated for data movement registered by schedule */
void optiq_schedule_mem_destroy(struct optiq_schedule *schedule, struct optiq_pami_transport *pami_transport);

/* Set variable for schedle */
void optiq_schedule_set(struct optiq_schedule *schedule, int world_size);

/* Return the currently used chunk size */
int optiq_schedule_get_chunk_size(struct optiq_job &ojob);

/* Print the jobs with current schedule */
void optiq_schedule_print_optiq_jobs (std::vector<struct optiq_job> &local_jobs);

/* Will do the follows:
 * 1. build next_dest look-up table to look for next dest for forwarding message.
 * 2. build notify list of end nodes and intermediate nodes to nofity that a path is done.
 * 3. register send/recv mem for the schedule
 * 4. create local optiq jobs.
 * 5. split the jobs into messages.
 * 6. set recv_length for the local rank.
 * */
void optiq_scheduler_build_schedule (void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls, std::vector<struct job> &jobs, std::vector<struct path *> &path_ranks);

/* 
 * Clear current schedule, some variable may also be clear 
 */
void optiq_schedule_clear();

/*
 * Free all schedule's memory and resources, ready to terminate 
 * */
void optiq_schedule_finalize();

#endif
