#ifndef OPTIQ_PAMI_TRANSPORT_H
#define OPTIQ_PAMI_TRANSPORT_H

#include <vector>
#include <queue>

using namespace std;

#ifdef __bgq__

#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>
#include <pami.h>

#endif

#include "message.h"

#define JOB_DONE_NOTIFICATION_DISPATCH_ID 16
#define RECV_MESSAGE_DISPATCH_ID 17

#define MAX_SHORT_MESSAGE_LENGTH 128

#define NUM_SEND_COOKIES 64
#define NUM_RECV_COOKIES 64

#define RECV_MESSAGE_SIZE (64*1024)
#define NUM_RECV_MESSAGES 1024
#define NUM_SEND_MESSAGES 1024

struct optiq_pami_transport;

struct optiq_send_cookie {
    struct optiq_message *message;
    struct optiq_pami_transport *pami_transport;
};

struct optiq_recv_cookie {
    struct optiq_message *message;
    struct optiq_pami_transport *pami_transport;
};

struct optiq_pami_transport {
    vector<struct optiq_recv_cookie *> avail_recv_cookies;
    vector<struct optiq_recv_cookie *> in_use_recv_cookies;

    vector<struct optiq_send_cookie *> avail_send_cookies;
    vector<struct optiq_send_cookie *> in_use_send_cookies;

    vector<struct optiq_message *> local_messages;

    vector<struct optiq_message *> in_use_recv_messages;
    vector<struct optiq_message *> avail_recv_messages;
    vector<struct optiq_message *> avail_send_messages;

    vector<int> involved_job_ids;
    vector<int> involved_task_ids;

    //vector<struct optiq_job> *jobs;
    int node_id;
    int rank;
    int size;
    size_t num_contexts;

#ifdef __bgq__
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;
#endif

};

void optiq_pami_transport_init(struct optiq_pami_transport *self);

int optiq_pami_transport_send(struct optiq_pami_transport *self, struct optiq_message *message);

int optiq_pami_transport_actual_send(struct optiq_pami_transport *self, struct optiq_message *message);

int optiq_pami_transport_recv(struct optiq_pami_transport *self, struct optiq_message *message);

bool optiq_pami_transport_test(struct optiq_pami_transport *self, struct optiq_message *message);

int optiq_pami_transport_process_incomming_message(struct optiq_pami_transport *self);

int optiq_notify_job_done(struct optiq_pami_transport *self, vector<int> *dests); 

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

void optiq_recv_job_done_notification_fn (
	pami_context_t    context,      /**< IN: PAMI context */
        void            *cookie,       /**< IN: dispatch cookie */
        const void      *header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      *data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     *recv);        /**< OUT: receive message structure */

#endif

int calculate_winsize(int message_size);
struct optiq_send_cookie* optiq_pami_transport_get_send_cookie(struct optiq_pami_transport *self);

struct optiq_message* optiq_pami_transport_get_send_message(struct optiq_pami_transport *self);
void optiq_pami_transport_return_send_message(struct optiq_pami_transport *self, struct optiq_message *message);


#endif
