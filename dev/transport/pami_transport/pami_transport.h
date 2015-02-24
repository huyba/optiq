#ifndef OPTIQ_PAMI_TRANSPORT_H
#define OPTIQ_PAMI_TRANSPORT_H

#include <vector>

#ifdef __bgq__

#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#include <pami.h>

#endif

#include "optiq_struct.h"
#include "schedule.h"

#define OPTIQ_MEM_REQUEST 0
#define OPTIQ_MEM_RESPONSE 1

#define MR_DESTINATION_REQUEST 9
#define MR_FORWARD_REQUEST 10
#define MR_RESPONSE 11
#define OPTIQ_RPUT_DONE 12
#define RPUT_DONE 13
#define RECV_MESSAGE 14
#define JOB_DONE 15

#define OPTIQ_NUM_RPUT_COOKIES (1024 * 1024)
#define OPTIQ_NUM_MESSAGE_HEADERS (1024* 1024)

#define MAX_SHORT_MESSAGE_LENGTH 128

#define OPTIQ_FORWARD_BUFFER_SIZE (256 * 1024 * 1024)

struct optiq_pami_transport;
struct optiq_schedule;

struct optiq_mem_request {
    int message_id;
    int source_id;
    int length;
};

struct optiq_mem_response {
    int dest_rank;
    pami_memregion_t mr;
    int offset;
    int message_id;
};

struct optiq_rput_cookie {
    struct optiq_pami_transport *pami_transport;
    struct optiq_message_header *message_header;
    int dest;
};

struct optiq_transport_info {
    bool initialized;
    bool finalized;
    char *forward_buf;
    struct optiq_memregion *forward_mr;

    std::vector<struct optiq_rput_cookie *> rput_cookies;
    std::vector<struct optiq_rput_cookie *> complete_rputs;

    std::vector<struct optiq_message_header *> forward_headers;
    std::vector<struct optiq_message_header *> message_headers;
    std::vector<struct optiq_message_header *> send_headers;
    std::vector<struct optiq_message_header *> processing_headers;

    std::vector<struct optiq_memregion> mr_responses;

    std::vector<struct optiq_mem_request> mem_requests;
    std::vector<struct optiq_mem_response> mem_responses;
    std::vector<int> rput_done;

    int global_header_id;
};

struct optiq_pami_transport {
    int rank;
    int size;
    size_t num_contexts;
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;

    struct optiq_transport_info transport_info;
    struct optiq_schedule *sched;
};

extern "C" struct optiq_pami_transport *pami_transport;

void optiq_pami_transport_init();

void optiq_transport_info_init(struct optiq_pami_transport *pami_transport);

void optiq_pami_transport_execute(struct optiq_pami_transport *pami_transport);

int optiq_pami_transport_finalize();

void optiq_transport_info_finalize(struct optiq_pami_transport *pami_transport);

struct optiq_pami_transport* optiq_pami_transport_get();

void optiq_pami_decrement (pami_context_t context, void *cookie, pami_result_t result);

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie, void (*rput_done_fn)(void*, void*, pami_result_t), void (*rput_rdone_fn)(void*, void*, pami_result_t));

int optiq_pami_send_immediate(pami_context_t &context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t &endpoint);

int optiq_pami_transport_send(void *buf, int count, int dest);

int optiq_pami_transport_recv(void *buf, int count, int source);

int optiq_pami_transport_rput(void *local_buf, int rput_bytes, int local_rank, void *remote_buf, int remote_rank);

void optiq_recv_mem_request_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_mem_response_fn (
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

void optiq_recv_rput_done_fn (
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


#endif
