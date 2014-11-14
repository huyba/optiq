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
#include "../../../core/structures/job.h"

extern struct optiq_transport_interface optiq_pami_transport_implementation;

#define RECV_MESSAGE_DISPATCH_ID 17
#define MAX_SHORT_MESSAGE_LENGTH 128

#define NUM_SEND_COOKIES 64
#define NUM_RECV_COOKIES 64

#define NUM_MESSAGES 64
#define MESSAGE_SIZE (2*1024*1024)

struct optiq_send_cookie;

struct optiq_send_cookie {
    struct optiq_message *message;
    vector<struct optiq_send_cookie *> *sent;
};

struct optiq_recv_cookie {
    struct optiq_message *message;
    vector<struct optiq_recv_cookie *> *received;
};

struct optiq_pami_transport {
#ifdef __bgq__
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;
    vector<struct optiq_recv_cookie *> avail_recv_cookies;
    vector<struct optiq_recv_cookie *> in_use_recv_cookies;
    vector<struct optiq_send_cookie *> avail_send_cookies;
    vector<struct optiq_send_cookie *> in_use_send_cookies;
    vector<struct optiq_message *> *in_use_messages;
    vector<struct optiq_message *> *avail_messages;
    vector<struct optiq_message *> *messages_no_buffer;

    vector<struct optiq_job> *jobs;
    int node_id;
    int rank;
    int size;
#endif
    size_t num_contexts;
};

void optiq_pami_transport_init(struct optiq_transport *self);

int optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message *message);

int optiq_pami_transport_recv(struct optiq_transport *self, struct optiq_message *message);

bool optiq_pami_transport_test(struct optiq_transport *self, struct optiq_job *job);

int optiq_pami_transport_destroy(struct optiq_transport *self);

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
