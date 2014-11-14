#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "job.h"
#include "pami_transport.h"

struct optiq_transport_interface optiq_pami_transport_implementation = {
    /*.init = */optiq_pami_transport_init,
    /*.send = */optiq_pami_transport_send,
    /*.recv = */optiq_pami_transport_recv,
    /*.test = */optiq_pami_transport_test,
    /*.destroy = */optiq_pami_transport_destroy   
};

void optiq_pami_transport_init(struct optiq_transport *self)
{
#ifdef __bgq__
    const char client_name[] = "OPTIQ";
    struct optiq_pami_transport *pami_transport;
    int configuration_count;
    pami_result_t result;
    pami_configuration_t *configurations;
    pami_configuration_t query_configurations[3];
    size_t contexts;

    configuration_count = 0;
    configurations = NULL;

    pami_transport = (struct optiq_pami_transport *) optiq_transport_get_concrete_transport(self);
    pami_transport->num_contexts = 1;

    /*
    * Create client
    */
    result = PAMI_Client_create(client_name, &pami_transport->client, configurations, configuration_count);

    assert(result == PAMI_SUCCESS);

    if (result != PAMI_SUCCESS) {
        return;
    }

    /*
    * Create context
    */
    result = PAMI_Context_createv(pami_transport->client, configurations, configuration_count, &pami_transport->context, pami_transport->num_contexts);
    assert(result == PAMI_SUCCESS);

    if (result != PAMI_SUCCESS) {
        return;
    }

    query_configurations[0].name = PAMI_CLIENT_NUM_TASKS;
    query_configurations[1].name = PAMI_CLIENT_TASK_ID;
    query_configurations[2].name = PAMI_CLIENT_NUM_CONTEXTS;

    result = PAMI_Client_query(pami_transport->client, query_configurations, 3);
    self->size = query_configurations[0].value.intval;
    self->rank = query_configurations[1].value.intval;
    contexts = query_configurations[2].value.intval;

    assert(contexts >= 1);

    /*Create endpoint for communication*/
    pami_transport->endpoints = (pami_endpoint_t *)malloc(sizeof(pami_endpoint_t) * self->size);
    for (int i = 0; i < self->size; i++) {
        PAMI_Endpoint_create(pami_transport->client, i, 0, &pami_transport->endpoints[i]);
    }

    /*
    * Register dispatch IDs
    */
    pami_dispatch_callback_function fn;
    pami_dispatch_hint_t options = {};
    fn.p2p = optiq_recv_message_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
            RECV_MESSAGE_DISPATCH_ID,
            fn,
            (void *) pami_transport,
            options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return;
    }

    /*Other initialization*/
    pami_transport->jobs = self->jobs;
    pami_transport->avail_messages = &self->avail_messages;
    pami_transport->in_use_messages = &self->in_use_messages;
    pami_transport->messages_no_buffer = &self->messages_no_buffer;

    /*Prepare cookies for sending*/
    struct optiq_send_cookie *send_cookies = (struct optiq_send_cookie *)malloc(sizeof(struct optiq_send_cookie) * NUM_SEND_COOKIES);
    if (send_cookies == NULL) {
        printf("Memory alloc error for send_cookies\n");
    }

    for (int i = 0; i < NUM_SEND_COOKIES; i++) {
        send_cookies[i].sent = &(pami_transport->in_use_send_cookies);
        printf("Rank %d add %dth element into send_cookies\n", self->rank, i);
        (pami_transport->avail_send_cookies).push_back((struct optiq_send_cookie *)&send_cookies[i]);
    }

    /*Prepare cookies for receiving*/
    struct optiq_recv_cookie *recv_cookies = (struct optiq_recv_cookie *)malloc(sizeof(struct optiq_recv_cookie) * NUM_RECV_COOKIES);
    for (int i = 0; i < NUM_RECV_COOKIES; i++) {
        recv_cookies[i].received = &(pami_transport->in_use_recv_cookies);
        pami_transport->avail_recv_cookies.push_back(&recv_cookies[i]);
    }   
#endif
}

int optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message *message)
{
    printf("Transport data of size %d to dest %d with flow_id = %d\n", message->length, message->next_dest, message->header.flow_id);
#ifdef __bgq__
    pami_result_t result;
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    struct optiq_send_cookie *send_cookie;

    if (pami_transport->avail_send_cookies.size() > 0) {
        send_cookie = pami_transport->avail_send_cookies.back();
        pami_transport->avail_send_cookies.pop_back();
    } else {
        send_cookie = (struct optiq_send_cookie *)malloc(sizeof(struct optiq_send_cookie));
        send_cookie->sent = &pami_transport->in_use_send_cookies;
    }

    send_cookie->message = message;

    if (message->length <= MAX_SHORT_MESSAGE_LENGTH) {
        pami_send_immediate_t parameter;
        parameter.dispatch = RECV_MESSAGE_DISPATCH_ID;
        parameter.header.iov_base = (void *)&message->header;
        parameter.header.iov_len = sizeof(struct optiq_message_header);
        parameter.data.iov_base = (void *)message->buffer;
        parameter.data.iov_len = message->length;
        parameter.dest = pami_transport->endpoints[message->next_dest];

        result = PAMI_Send_immediate (pami_transport->context, &parameter);
        assert(result == PAMI_SUCCESS);
        if (result != PAMI_SUCCESS) {
            return 1;
        }

        /*Add the cookie to the queue of sent messages*/
        (*send_cookie->sent).push_back(send_cookie);
    } else {
        pami_send_t param_send;
        param_send.send.dest = message->next_dest;
        param_send.send.dispatch = RECV_MESSAGE_DISPATCH_ID;
        param_send.send.header.iov_base = (void *)&message->header;
        param_send.send.header.iov_len = sizeof(struct optiq_message_header);
        param_send.send.data.iov_base = (void *)message->buffer;
        param_send.send.data.iov_len = message->length;
        param_send.events.cookie = (void *)&send_cookie;
        param_send.events.local_fn = optiq_send_done_fn;
        param_send.events.remote_fn = NULL;

        result = PAMI_Send(pami_transport->context, &param_send);
        assert(result == PAMI_SUCCESS);
        if (result != PAMI_SUCCESS) {
            return 1;
        }
    }
#endif
    return 0;
}

int optiq_pami_transport_destroy(struct optiq_transport *self)
{

}

int optiq_pami_transport_recv(struct optiq_transport *self, struct optiq_message *message)
{

}

bool optiq_pami_transport_test(struct optiq_transport *self, struct optiq_job *job)
{
    bool isDone = true;
#ifdef __bgq__
    pami_result_t result;
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    PAMI_Context_advance (pami_transport->context, 100);
    optiq_send_cookie *send_cookie;

    /*Return cookie, messag back to available queues. Adding message sent to flow's*/
    while (pami_transport->in_use_send_cookies.size() > 0) {
        send_cookie = pami_transport->in_use_send_cookies.back();
        pami_transport->in_use_send_cookies.pop_back();
        pami_transport->avail_send_cookies.push_back(send_cookie);

        (*pami_transport->messages_no_buffer).push_back(send_cookie->message);
    
        for (int i = 0; i < job->flows.size(); i++) {
            if (send_cookie->message->header.flow_id == job->flows[i].id) {
                job->flows[i].sent_bytes == send_cookie->message->length;
            }
        }
    }

    /*Checking if every flow is done*/
    for (int i = 0; i < job->flows.size(); i++) {
        if (job->flows[i].message->length != job->flows[i].sent_bytes) {
            isDone = false;
        }
    }

#endif
    return isDone;
}

/*
 * Process the incomming message. Either forward it by putting it in a virtual lane or keep it
 * */
int optiq_pami_transport_process_incomming_message(vector<struct optiq_recv_cookie *> *received)
{

}

#ifdef __bgq__
void optiq_recv_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    struct optiq_recv_cookie *recv_cookie = (struct optiq_recv_cookie *)cookie;
    (*recv_cookie->received).push_back(recv_cookie);

    optiq_pami_transport_process_incomming_message(recv_cookie->received);
}

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    /*Add send cookie into in_use_cookie*/
    struct optiq_send_cookie *send_cookie = (struct optiq_send_cookie *)cookie;
    (*send_cookie->sent).push_back(send_cookie);
}

void optiq_recv_message_fn(pami_context_t context, void *cookie, const void *header, size_t header_size,
                const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *) cookie;
    struct optiq_message_header *message_header = (struct optiq_message_header *) header;


    struct optiq_message *message;

    /*If incomming message is larger than the default size, need to allocate new memory*/
    if (data_size > MESSAGE_SIZE) {
        message = (struct optiq_message *)malloc(sizeof(struct optiq_message));
        message->buffer = (char *)malloc(data_size);
    } else {
        /*If there is still available message to use*/
        if ((*pami_transport->avail_messages).size() > 0) {
            message = (*pami_transport->avail_messages).back();
            (*pami_transport->avail_messages).pop_back();
        } else {
            message = (struct optiq_message *)malloc(sizeof(struct optiq_message));
            message->buffer = (char *)malloc(MESSAGE_SIZE);
        }
    }

    memcpy(&message->header, header, sizeof(struct optiq_message_header));
    message->next_dest = get_next_dest_from_jobs(*(pami_transport->jobs), message->header.flow_id, pami_transport->node_id);
    message->length = data_size;
    message->current_offset = 0;

    struct optiq_recv_cookie *recv_cookie;

    if (pami_transport->avail_recv_cookies.size() > 0) {
        recv_cookie = pami_transport->avail_recv_cookies.back();
        pami_transport->avail_recv_cookies.pop_back();
    } else {
        recv_cookie = (struct optiq_recv_cookie *) malloc(sizeof(struct optiq_recv_cookie));
        recv_cookie->received = &pami_transport->in_use_recv_cookies;
    }

    recv_cookie->message = message;

    if (data != NULL) {
        memcpy(message->buffer, data, data_size);
        (*recv_cookie->received).push_back(recv_cookie);
        optiq_pami_transport_process_incomming_message(recv_cookie->received);
    } else {
        recv->local_fn = optiq_recv_done_fn;
        recv->cookie = (void *)recv_cookie;
        recv->type = PAMI_TYPE_BYTE;
        recv->addr = (void *)message->buffer;
        recv->offset = 0;
        recv->data_fn = PAMI_DATA_COPY;
        recv->data_cookie = NULL;
    }
    return;
}
#endif
