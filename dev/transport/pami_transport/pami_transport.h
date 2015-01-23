#ifndef OPTIQ_PAMI_TRANSPORT
#define OPTIQ_PAMI_TRANSPORT

#include <vector>
#include <pami.h>

#define OPTIQ_MEM_REQUEST 0
#define OPTIQ_MEM_RESPONSE 1

#define OPTIQ_RPUT_DONE 11

struct optiq_mem_request {
    int source_rank;
    int nbytes;
};

struct optiq_mem_response {
    int dest_rank;
    pami_memregion_t mr;
    int offset;
    int message_id;
};

struct optiq_pami_transport {
    int rank;
    int size;
    size_t num_contexts;
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;

    std::vector<struct optiq_mem_request> mem_requests;
    std::vector<struct optiq_mem_response> mem_responses;

    int message_id;
    std::vector<int> rput_done;
};

extern "C" struct optiq_pami_transport *pami_transport;

void optiq_pami_transport_init();

struct optiq_pami_transport* optiq_get_pami_transport();

void optiq_pami_rput_done_fn(pami_context_t context, void *cookie, pami_result_t result);

void optiq_pami_rput_rdone_fn(pami_context_t context, void *cookie, pami_result_t result);

void decrement (pami_context_t context, void *cookie, pami_result_t result);

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie, void *rput_done_fn, void *rput_rdone_fn);

int optiq_pami_send_immediate(pami_context_t &context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t &endpoint);

void optiq_recv_mr_response_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

void optiq_recv_mr_request_fn (
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

#endif
