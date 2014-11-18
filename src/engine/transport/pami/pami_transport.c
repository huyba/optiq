#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "job.h"
#include "memory.h"
#include "virtual_lane.h"
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
    pami_result_t result;
    pami_configuration_t query_configurations[3];
    size_t contexts;

    int configuration_count = 0;
    pami_configuration_t *configurations = NULL;

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
    pami_transport->rank = self->rank;
    pami_transport->size = self->size;
    pami_transport->node_id = self->rank;
    pami_transport->virtual_lanes = self->virtual_lanes;

    assert(contexts >= 1);

    /*Create endpoint for communication*/
    pami_transport->endpoints = (pami_endpoint_t *)core_memory_alloc(sizeof(pami_endpoint_t) * self->size, "endpoints", "pami_init");
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
    pami_transport->avail_recv_messages = &self->avail_recv_messages;
    pami_transport->in_use_recv_messages = &self->in_use_recv_messages;
    pami_transport->avail_send_messages = &self->avail_send_messages;

    /*Prepare cookies for sending*/
    for (int i = 0; i < NUM_SEND_COOKIES; i++) {
        struct optiq_send_cookie *send_cookie = (struct optiq_send_cookie *)core_memory_alloc(sizeof(struct optiq_send_cookie), "send_cookies", "pami_init");
        send_cookie->pami_transport = pami_transport;
        pami_transport->avail_send_cookies.push_back(send_cookie);
    }

    /*Prepare cookies for receiving*/
    for (int i = 0; i < NUM_RECV_COOKIES; i++) {
        struct optiq_recv_cookie *recv_cookie = (struct optiq_recv_cookie *)core_memory_alloc(sizeof(struct optiq_recv_cookie), "recv_cookies", "pami_init");
        recv_cookie->pami_transport = pami_transport;
        pami_transport->avail_recv_cookies.push_back(recv_cookie);
    }
#endif
}

int optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message *message)
{
    printf("Rank %d sends data of size %d to dest %d with flow_id = %d\n", self->rank, message->length, message->next_dest, message->header.flow_id);
#ifdef __bgq__
    pami_result_t result;
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    struct optiq_send_cookie *send_cookie;

    if (pami_transport->avail_send_cookies.size() > 0) {
        send_cookie = pami_transport->avail_send_cookies.back();
        pami_transport->avail_send_cookies.pop_back();
    } else {
        send_cookie = (struct optiq_send_cookie *)core_memory_alloc(sizeof(struct optiq_send_cookie), "send_cookies", "pami_transport_send");
        send_cookie->pami_transport = pami_transport;
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

        /*Add the cookie to the vector of in-use send cookies*/
        pami_transport->in_use_send_cookies.push_back(send_cookie);
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
#ifdef __bgq__
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    /*Free endpoint*/
    free(pami_transport->endpoints);

    /*Free recv_cookies*/
    struct optiq_recv_cookie *recv_cookie;
    while (pami_transport->avail_recv_cookies.size() > 0) {
        recv_cookie = pami_transport->avail_recv_cookies.back();
        pami_transport->avail_recv_cookies.pop_back();
        free(recv_cookie);
    }

    /*Free send_cookies*/
    struct optiq_send_cookie *send_cookie;
    while (pami_transport->avail_send_cookies.size() > 0) {
        send_cookie = pami_transport->avail_send_cookies.back();
        pami_transport->avail_send_cookies.pop_back();
        free(send_cookie);
    }

    return 0;
#endif
}

/* Return 1 if entire message is ready, 0 if not*/
int optiq_pami_transport_recv(struct optiq_transport *self, struct optiq_message *message)
{
#ifdef __bgq__
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    PAMI_Context_advance (pami_transport->context, 100);

    for (int i = 0; i < pami_transport->local_messages.size(); i++) {
        struct optiq_message *instant = pami_transport->local_messages.back();

        printf("Rank %d received as the destination of a message of size %d\n", pami_transport->rank, instant->length);

        if (instant->header.flow_id == message->header.flow_id) {
            memcpy((void *)message->buffer[instant->header.original_offset], (const void*)instant->buffer, instant->length);
            message->recv_length += instant->length;

            pami_transport->local_messages.pop_back();
            (*pami_transport->avail_recv_messages).push_back(instant);

            if (message->recv_length == instant->header.original_length) {
                return 1;
            }
        }
    }

#endif
    return 0;
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
    
        for (int i = 0; i < job->flows.size(); i++) {
            if (send_cookie->message->header.flow_id == job->flows[i].id) {
                job->flows[i].sent_bytes == send_cookie->message->length;
            }
        }

        pami_transport->in_use_send_cookies.pop_back();
        pami_transport->avail_send_cookies.push_back(send_cookie);
        (*(pami_transport->avail_send_messages)).push_back(send_cookie->message);
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
int optiq_pami_transport_process_incomming_message(struct optiq_pami_transport *pami_transport)
{
    if (pami_transport->in_use_recv_cookies.size() > 0) {
        struct optiq_recv_cookie *recv_cookie = pami_transport->in_use_recv_cookies.back();
        struct optiq_message *message = recv_cookie->message;

        /*If the final destination is at local, deliver it*/
        if (message->header.final_dest == pami_transport->rank) {
            pami_transport->local_messages.push_back(message);
        } 
        /*If the final destination is at other node, put the message to the virtual lane*/
        else {
            message->next_dest = get_next_dest_from_jobs(*(pami_transport->jobs), message->header.flow_id, pami_transport->node_id);
            message->source = pami_transport->rank;
            add_message_to_virtual_lanes(message, pami_transport->virtual_lanes);
        }

        /*Free the recv_cookie*/
        pami_transport->in_use_recv_cookies.pop_back();
        pami_transport->avail_recv_cookies.push_back(recv_cookie);
    }

    return 0;
}

#ifdef __bgq__
void optiq_recv_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    struct optiq_recv_cookie *recv_cookie = (struct optiq_recv_cookie *)cookie;
    recv_cookie->pami_transport->in_use_recv_cookies.push_back(recv_cookie);

    optiq_pami_transport_process_incomming_message(recv_cookie->pami_transport);
}

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    /*Add send cookie into in_use_cookie*/
    struct optiq_send_cookie *send_cookie = (struct optiq_send_cookie *)cookie;
    send_cookie->pami_transport->in_use_send_cookies.push_back(send_cookie);
}

void optiq_recv_message_fn(pami_context_t context, void *cookie, const void *header, size_t header_size,
                const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *) cookie;
    struct optiq_message_header *message_header = (struct optiq_message_header *) header;

    printf("At %d received from %d with %d bytes\n", pami_transport->rank, origin, data_size);

    struct optiq_message *message;

    /*If incomming message is larger than the default size, need to allocate new memory*/
    if (data_size > RECV_MESSAGE_SIZE) {
        message = get_message_with_buffer(data_size);
    } else {
        /*If there is still available message to use*/
        if ((*pami_transport->avail_recv_messages).size() > 0) {
            message = (*pami_transport->avail_recv_messages).back();
            (*pami_transport->avail_recv_messages).pop_back();
        } else {
            message = get_message_with_buffer(RECV_MESSAGE_SIZE);
        }
    }

    memcpy(&message->header, header, sizeof(struct optiq_message_header));
    message->length = data_size;
    message->current_offset = 0;

    struct optiq_recv_cookie *recv_cookie;

    if (pami_transport->avail_recv_cookies.size() > 0) {
        recv_cookie = pami_transport->avail_recv_cookies.back();
        pami_transport->avail_recv_cookies.pop_back();
    } else {
        recv_cookie = (struct optiq_recv_cookie *) core_memory_alloc(sizeof(struct optiq_recv_cookie), "recv_cookie", "recv_message_fn");
        recv_cookie->pami_transport = pami_transport;
    }

    recv_cookie->message = message;

    if (data != NULL) {
        memcpy(message->buffer, data, data_size);
        pami_transport->in_use_recv_cookies.push_back(recv_cookie);
        optiq_pami_transport_process_incomming_message(pami_transport);
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
