#ifndef OPTIQ_PAMI_TRANSPORT_H
#define OPTIQ_PAMI_TRANSPORT_H

#ifdef __bgq__
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#include <pami.h>
#endif

#include "../transport_interface.h"
#include "../../message.h"

extern struct optiq_transport_interface optiq_pami_transport_implementation;

#define RECV_MESSAGE_DISPATCH_ID 17
#define MAX_SHORT_MESSAGE_LENGTH 128

struct optiq_send_cookie {

};

struct optiq_recv_cookie {
    struct optiq_message message;
    vector<struct optiq_recv_cookie *> receives;
};

struct optiq_pami_transport {
#ifdef __bgq__
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;
    vector<struct optiq_recv_cookie *> avail_recv_cookies;
    vector<struct optiq_recv_cookie *> in_use_recv_cookies;
    vector<struct optiq_job> *jobs;
    int node_id;
    int rank;
    int size;
#endif
    size_t num_contexts;
};

void optiq_pami_transport_init(struct optiq_transport *self);

int optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message &message);

#ifdef __bgq__
void optiq_recv_done_fn(pami_context_t context, void *cookie, pami_result_t result);

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result);

void optiq_recv_message_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */
#endif

#endif
