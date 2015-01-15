#ifndef OPTIQ_PAMI_TRANSPORT_H
#define OPTIQ_PAMI_TRANSPORT_H

#include <vector>

#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#include <pami.h>

#include "multibfs.h"

#define MR_DESTINATION_REQUEST 9
#define MR_FORWARD_REQUEST 10
#define MR_RESPONSE 11
#define RPUT_DONE 12
#define RECV_MESSAGE 13
#define JOB_DONE 14

#define OPTIQ_NUM_RPUT_COOKIES (32 * 1024)
#define OPTIQ_NUM_MESSAGE_HEADERS (32* 1024)

#define MAX_SHORT_MESSAGE_LENGTH 128

#define OPTIQ_FORWARD_BUFFER_SIZE (128 * 1024 * 1024)

struct optiq_pami_transport;

struct optiq_memregion {
    pami_memregion_t mr;
    int offset;
    int header_id;
};

struct optiq_message_header {
    int length;
    int source;
    int dest;
    int flow_id;
    struct optiq_memregion mem;
    int header_id;
    int original_offset;
};

struct optiq_rput_cookie {
    struct optiq_pami_transport *pami_transport;
    struct optiq_message_header *message_header;
    int dest;
};

struct optiq_bulk {
    int bulkd_id;

    int world_rank;

    int remaining_jobs;
    int *next_dest;

    int expecting_length;
    int sent_bytes;
    int *recv_bytes;

    bool isDest;
    bool isSource;

    int *rdispls;

    struct optiq_memregion recv_mr;
    struct optiq_memregion send_mr;

    struct optiq_pami_transport *pami_transport;

    struct multibfs *bfs;

    int *flow_id;
    int *final_dest;
};

struct optiq_pami_extra {
    struct optiq_memregion *forward_mr;

    std::vector<struct optiq_rput_cookie *> rput_cookies;
    std::vector<struct optiq_rput_cookie *> complete_rputs;

    std::vector<struct optiq_message_header *> forward_headers;
    std::vector<struct optiq_message_header *> message_headers;
    std::vector<struct optiq_message_header *> send_headers;
    std::vector<struct optiq_message_header *> processing_headers;

    std::vector<struct optiq_memregion> mr_responses;

    int global_header_id;
};

struct optiq_pami_transport {
    int rank;
    int size;
    size_t num_contexts;
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;

    struct optiq_pami_extra extra;
    struct optiq_bulk bulk;
};

void optiq_pami_init(struct optiq_pami_transport *self);
void optiq_pami_init_extra(struct optiq_pami_transport *self);

void optiq_execute_jobs(struct optiq_pami_transport *pami_transport);

int optiq_pami_send_immediate(pami_context_t &context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t &endpoint);

void optiq_pami_rput_done_fn(pami_context_t context, void *cookie, pami_result_t result);

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie);

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result);

int optiq_pami_send(pami_context_t context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t endpoint, void *cookie);

void optiq_recv_message_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_job_done_notification_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_rput_done_notification_fn (
	pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_mr_response_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_mr_forward_request_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_mr_destination_request_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

#endif
