#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "memory.h"
#include "pami_transport.h"

struct optiq_transport_interface optiq_pami_transport_implementation = {
    /*.init = */optiq_pami_transport_init,
    /*.send = */optiq_pami_transport_send       
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
    
#endif
}

int optiq_pami_transport_send(struct optiq_transport *self, struct optiq_message &message)
{
    printf("Transport data of size %d to dest %d with flow_id = %d\n", message.length, message.next_dest, message.header.flow_id);
#ifdef __bgq__
    pami_result_t result;
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(self);

    struct optiq_send_cookie send_cookie;

    if (message.length <= MAX_SHORT_MESSAGE_LENGTH) {
        pami_send_immediate_t parameter;
        parameter.dispatch = RECV_MESSAGE_DISPATCH_ID;
        parameter.header.iov_base = (void *)&message.header;
        parameter.header.iov_len = sizeof(struct optiq_message_header);
        parameter.data.iov_base = (void *)message.buffer;
        parameter.data.iov_len = message.length;
        parameter.dest = pami_transport->endpoints[message.next_dest];

        result = PAMI_Send_immediate (pami_transport->context, &parameter);
        assert(result == PAMI_SUCCESS);
        if (result != PAMI_SUCCESS) {
            return 1;
        }
    } else {
        pami_send_t param_send;
        param_send.send.dest = message.next_dest;
        param_send.send.dispatch = RECV_MESSAGE_DISPATCH_ID;
        param_send.send.header.iov_base = (void *)&message.header;
        param_send.send.header.iov_len = sizeof(struct optiq_message_header);
        param_send.send.data.iov_base = (void *)message.buffer;
        param_send.send.data.iov_len = message.length;
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

#ifdef __bgq__
void optiq_recv_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{

}

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{

}

void optiq_recv_message_fn(pami_context_t context, void *cookie, const void *header, size_t header_size,
                const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *) cookie;
    struct optiq_message_header *message_header = (struct optiq_message_header *) header;
    struct optiq_message message;
    memcpy(&message.header, header, sizeof(struct optiq_message_header));
    message.buffer = (char *)core_memory_pool_allocate(data_size);
    message.next_dest = get_next_dest_from_jobs(pami_transport->jobs, message.header.flow_id, pami_transport->node_id);
    message.length = data_size;
    message.current_offset = 0;

    struct optiq_recv_cookie *recv_cookie;

    if (pami_transport->avail_recv_cookies.size() > 0) {
        recv_cookie = pami_transport->avail_recv_cookies.front();
        pami_transport->avail_recv_cookies.erase(pami_transport->avail_recv_cookies.begin());
    } else {
        recv_cookie = (struct optiq_recv_cookie *) malloc(sizeof(struct optiq_recv_cookie));
    }

    recv_cookie->receives = pami_transport->in_use_recv_cookies;
    recv_cookie->message = message;

    if (data != NULL) {
        memcpy(message.buffer, data, data_size);
        recv_cookie->receives.push_back(recv_cookie);   
    } else {
        recv->local_fn = optiq_recv_done_fn;
        recv->cookie = (void *)recv_cookie;
        recv->type = PAMI_TYPE_BYTE;
        recv->addr = (void *)message.buffer;
        recv->offset = 0;
        recv->data_fn = PAMI_DATA_COPY;
        recv->data_cookie = NULL;
    }
    return;
}
#endif
