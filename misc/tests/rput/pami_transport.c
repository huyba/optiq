#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "pami_transport.h"

void optiq_pami_init(struct optiq_pami_transport *pami_transport)
{
    const char client_name[] = "OPTIQ";
    pami_result_t result;
    pami_configuration_t query_configurations[3];
    size_t contexts;

    int configuration_count = 0;
    pami_configuration_t *configurations = NULL;
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
    pami_transport->size = query_configurations[0].value.intval;
    pami_transport->rank = query_configurations[1].value.intval;
    contexts = query_configurations[2].value.intval;

    assert(contexts >= 1);

    /*Create endpoint for communication*/
    pami_transport->endpoints = (pami_endpoint_t *)malloc(sizeof(pami_endpoint_t) * pami_transport->size);
    for (int i = 0; i < pami_transport->size; i++) {
	PAMI_Endpoint_create(pami_transport->client, i, 0, &pami_transport->endpoints[i]);
    }

    /*
     * Register dispatch IDs
     */
    pami_dispatch_callback_function fn;
    pami_dispatch_hint_t options = {};

    /*Message has come notification*/
    fn.p2p = optiq_recv_message_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    RECV_MESSAGE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Job done notification*/
    fn.p2p = optiq_recv_rput_done_notification_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
            RPUT_DONE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    fn.p2p = optiq_recv_mr_request_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
            MR_REQUEST,
            fn,
            (void *) pami_transport,
            options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return;
    }

    fn.p2p = optiq_recv_mr_response_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
            MR_RESPONSE,
            fn,
            (void *) pami_transport,
            options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return;
    }
}

void optiq_pami_rput_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    int *val = (int *)cookie;
    (*val)--;   
}

void optiq_pami_rput_rdone_fn(pami_context_t context, void *cookie, pami_result_t result)
{   
    struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)cookie;
    rput_cookie->val--;
}

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie)
{
    int ret = 0;

    pami_rput_simple_t parameters;

    parameters.rma.hints          = (pami_send_hint_t) {0};
    parameters.rma.cookie         = cookie;

    parameters.rma.done_fn        = optiq_pami_rput_done_fn;
    parameters.rdma.local.mr      = local_mr;
    parameters.rdma.local.offset  = local_offset;
    parameters.rma.bytes      = nbytes;

    parameters.rma.dest = endpoint;

    parameters.rdma.remote.mr = remote_mr;
    parameters.rdma.remote.offset = remote_offset;
    parameters.put.rdone_fn = optiq_pami_rput_rdone_fn;

    pami_result_t result = PAMI_Rput (context, &parameters);
    if (result != PAMI_SUCCESS) {
        printf("Error in PAMI_Put\n");
    }

    return ret;
}

int optiq_pami_send_immediate(pami_context_t &context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t &endpoint)
{
    int ret = 0;

    pami_send_immediate_t parameter;
    parameter.dispatch = dispatch;
    parameter.header.iov_base = header_base;
    parameter.header.iov_len = header_len;
    parameter.data.iov_base = data_base;
    parameter.data.iov_len = data_len;
    parameter.dest = endpoint;

    pami_result_t result = PAMI_Send_immediate (context, &parameter);
    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return 1;
    }

    return ret;
}

void optiq_send_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    int *val = (int *)cookie;
    (*val)--;
}

int optiq_pami_send(pami_context_t context, int dispatch, void *header_base, int header_len, void *data_base, int data_len, pami_endpoint_t endpoint, void *cookie)
{
    pami_send_t param_send;
    param_send.send.dest = endpoint;
    param_send.send.dispatch = dispatch;
    param_send.send.header.iov_base = header_base;
    param_send.send.header.iov_len = header_len;
    param_send.send.data.iov_base = data_base;
    param_send.send.data.iov_len = data_len;
    param_send.events.cookie = cookie;
    param_send.events.local_fn = optiq_send_done_fn;
    param_send.events.remote_fn = NULL;

    pami_result_t result = PAMI_Send(context, &param_send);
    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
        return 1;
    }

    return 0;
}

void optiq_recv_message_fn(pami_context_t context, void *cookie, const void *header, size_t header_size,
	const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{

}

void optiq_recv_rput_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    struct optiq_rput_cookie *rput_cookie = pami_transport->rput_cookie;
    rput_cookie->val--;
}

void optiq_recv_mr_response_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    memcpy(pami_transport->remote_mr, data, data_size);
    pami_transport->rput_cookie->mr_val--;
}

void optiq_recv_mr_request_fn(pami_context_t context, void *cookie, const void *header, size_t header_size,
        const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    optiq_pami_send_immediate(pami_transport->context, MR_RESPONSE, NULL, 0, pami_transport->remote_mr, sizeof(pami_memregion_t), pami_transport->endpoints[origin]);
}
