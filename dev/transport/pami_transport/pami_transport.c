#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include "pami_transport.h"

struct optiq_pami_transport *pami_transport;

void optiq_pami_transport_init()
{
    const char client_name[] = "OPTIQ";
    pami_result_t result;
    pami_configuration_t query_configurations[3];
    size_t contexts;

    int configuration_count = 0;
    pami_configuration_t *configurations = NULL;

    /*Init the pami_transport variable*/
    pami_transport = (struct optiq_pami_transport *) calloc(1, sizeof(struct optiq_pami_transport));

    /* Create client */
    result = PAMI_Client_create(client_name, &pami_transport->client, configurations, configuration_count);

    assert(result == PAMI_SUCCESS);

    if (result != PAMI_SUCCESS) {
	return;
    }

    /* Create context */
    pami_transport->num_contexts = 1;
    result = PAMI_Context_createv(pami_transport->client, configurations, configuration_count, &pami_transport->context, pami_transport->num_contexts);
    assert(result == PAMI_SUCCESS);

    if (result != PAMI_SUCCESS) {
	return;
    }

    /* Query necessary info*/
    query_configurations[0].name = PAMI_CLIENT_NUM_TASKS;
    query_configurations[1].name = PAMI_CLIENT_TASK_ID;
    query_configurations[2].name = PAMI_CLIENT_NUM_CONTEXTS;

    result = PAMI_Client_query(pami_transport->client, query_configurations, 3);
    pami_transport->size = query_configurations[0].value.intval;
    pami_transport->rank = query_configurations[1].value.intval;
    contexts = query_configurations[2].value.intval;

    assert(contexts >= 1);

    /* Create endpoint for communication */
    pami_transport->endpoints = (pami_endpoint_t *)malloc(sizeof(pami_endpoint_t) * pami_transport->size);
    for (int i = 0; i < pami_transport->size; i++) {
	PAMI_Endpoint_create(pami_transport->client, i, 0, &pami_transport->endpoints[i]);
    }

    /*Dispatch IDs register*/
    pami_dispatch_callback_function fn;
    pami_dispatch_hint_t options = {};

    /*Message has come notification*/
    fn.p2p = optiq_recv_mr_request_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    OPTIQ_MEM_REQUEST,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Receive memory response*/
    fn.p2p = optiq_recv_mr_response_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    OPTIQ_MEM_RESPONSE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    fn.p2p = optiq_recv_rput_done_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    OPTIQ_RPUT_DONE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Other initialization*/
    pami_transport->message_id = 0;
}

struct optiq_pami_transport* optiq_pami_transport_get()
{
    return pami_transport;
}

void optiq_pami_decrement (pami_context_t context, void *cookie, pami_result_t result)
{
    unsigned * value = (unsigned *) cookie;
    --*value;
}

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie, void (*rput_done_fn)(void*, void*, pami_result_t), void (*rput_rdone_fn)(void*, void*, pami_result_t))
{
    int ret = 0;

    pami_rput_simple_t parameters;

    parameters.rma.hints          = (pami_send_hint_t) {0};
    parameters.rma.cookie         = cookie;

    if (rput_done_fn == NULL) {
	parameters.rma.done_fn = NULL;
    } else {
	parameters.rma.done_fn        = (*rput_done_fn);
    }

    parameters.rdma.local.mr      = local_mr;
    parameters.rdma.local.offset  = local_offset;
    parameters.rma.bytes      = nbytes;

    parameters.rma.dest = endpoint;

    parameters.rdma.remote.mr = remote_mr;
    parameters.rdma.remote.offset = remote_offset;

    if (rput_rdone_fn == NULL) {
	parameters.put.rdone_fn = NULL;
    } else {
	parameters.put.rdone_fn = (*rput_rdone_fn);
    }

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

void optiq_recv_mr_request_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    struct optiq_mem_request request;
    request.source_rank = origin;
    request.nbytes = (*(int *)data);

    pami_transport->mem_requests.push_back(request);
}

void optiq_recv_mr_response_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    struct optiq_mem_response response;

    memcpy (&response, data, sizeof(struct optiq_mem_response));

    pami_transport->mem_responses.push_back(response);
}

void optiq_recv_rput_done_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    int message_id = (*(int *)data);

    pami_transport->rput_done.push_back(message_id);
}

int optiq_pami_transport_send (void *send_buf, int send_bytes, int dest_rank)
{
    int ret = 0;

    size_t bytes;
    pami_memregion_t send_mr;
    int send_offset = 0;

    /*Register its own memory*/
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, send_buf, send_bytes, &bytes, &send_mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < send_bytes) {
	printf("Registered less\n");
    }

    /*Wait until there is a mem region for it*/
    struct optiq_mem_response response;
    bool found = false;

    while (!found) {
	PAMI_Context_advance (pami_transport->context, 100);

	for (int i = 0; i < pami_transport->mem_responses.size(); i++) {
	    if (pami_transport->mem_responses[i].dest_rank == dest_rank) {
		response = pami_transport->mem_responses[i];
		pami_transport->mem_responses.erase (pami_transport->mem_responses.begin() + i);
		found = true;

		break;
	    }
	}
    }

    unsigned send_cookie = 1;

    /*Put data*/
    optiq_pami_rput(pami_transport->client, pami_transport->context, &send_mr, send_offset, send_bytes, pami_transport->endpoints[dest_rank], &response.mr, response.offset, &send_cookie, NULL, optiq_pami_decrement);

    /*Wait until the put is done at remote side*/
    while (send_cookie == 1) {
	PAMI_Context_advance (pami_transport->context, 100);
    }

    /*Notify that the rput is done*/
    optiq_pami_send_immediate (pami_transport->context, OPTIQ_RPUT_DONE, NULL, NULL, &response.message_id, sizeof(int), pami_transport->
	    endpoints[dest_rank]);

    /*Destroy the memregion was used*/
    result = PAMI_Memregion_destroy (pami_transport->context, &send_mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    }

    return ret;
}

int optiq_pami_transport_recv (void *recv_buf, int recv_bytes, int source_rank)
{
    int ret = 0;

    size_t bytes;

    /*Send the mem region back to the dest*/
    struct optiq_mem_response response;
    response.dest_rank = pami_transport->rank;
    response.offset = 0;
    response.message_id = pami_transport->message_id;
    pami_transport->message_id = (pami_transport->message_id++) % INT_MAX;

    /*Register for its own mem region*/
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, recv_buf, recv_bytes, &bytes, &response.mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < recv_bytes) {
	printf("Registered less\n");
    }

    optiq_pami_send_immediate (pami_transport->context, OPTIQ_MEM_RESPONSE, NULL, NULL, &response, sizeof(struct optiq_mem_response), pami_transport->endpoints[source_rank]);

    /*Wait until put is done*/
    bool done = false;

    while (!done) {
	PAMI_Context_advance (pami_transport->context, 100);

	for (int i = 0; i < pami_transport->rput_done.size(); i++) {
	    if (pami_transport->rput_done[i] == response.message_id) {
		done = true;
		pami_transport->rput_done.erase(pami_transport->rput_done.begin() + i);

		break;
	    }
	}
    }

    /*Destroy the mem region*/
    result = PAMI_Memregion_destroy (pami_transport->context, &response.mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    }

    return ret;
}

int optiq_pami_transport_rput(void *local_buf, int rput_bytes, int local_rank, void *remote_buf, int remote_rank)
{
    int ret = 0;

    if (pami_transport->rank == local_rank) 
    {
	size_t bytes;
	pami_memregion_t send_mr;
	int send_offset = 0;

	/*Register its own memory*/
        pami_result_t result = PAMI_Memregion_create (pami_transport->context, local_buf, rput_bytes, &bytes, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < rput_bytes) {
	    printf("Registered less\n");
	}

	/*Wait until there is a mem region for it*/
	struct optiq_mem_response response;
	bool found = false;

	while (!found) {
	    PAMI_Context_advance (pami_transport->context, 100);

	    for (int i = 0; i < pami_transport->mem_responses.size(); i++) {
		if (pami_transport->mem_responses[i].dest_rank == remote_rank) {
		    response = pami_transport->mem_responses[i];
		    pami_transport->mem_responses.erase (pami_transport->mem_responses.begin() + i);
		    found = true;

		    break;
		}
	    }
	}

	unsigned send_cookie = 1;

	/*Put data*/
	optiq_pami_rput(pami_transport->client, pami_transport->context, &send_mr, send_offset, rput_bytes, pami_transport->endpoints[remote_rank], &response.mr, response.offset, &send_cookie, NULL, optiq_pami_decrement);

	/*Wait until the put is done at remote side*/
	while (send_cookie == 1) {
	    PAMI_Context_advance (pami_transport->context, 100);
	}

	/*Notify that the rput is done*/
	optiq_pami_send_immediate (pami_transport->context, OPTIQ_RPUT_DONE, NULL, NULL, &response.message_id, sizeof(int), pami_transport->
endpoints[remote_rank]);

	/*Destroy the memregion was used*/
	result = PAMI_Memregion_destroy (pami_transport->context, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
    }

    if (pami_transport->rank == remote_rank)
    {
	size_t bytes;

	/*Send the mem region back to the dest*/
        struct optiq_mem_response response;
        response.dest_rank = pami_transport->rank;
        response.offset = 0;
        response.message_id = pami_transport->message_id;
        pami_transport->message_id = (pami_transport->message_id++) % INT_MAX;

	/*Register for its own mem region*/
	pami_result_t result = PAMI_Memregion_create (pami_transport->context, remote_buf, rput_bytes, &bytes, &response.mr);
    	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < rput_bytes) {
	    printf("Registered less\n");
	}

	optiq_pami_send_immediate (pami_transport->context, OPTIQ_MEM_RESPONSE, NULL, NULL, &response, sizeof(struct optiq_mem_response), pami_transport->endpoints[local_rank]);

	/*Wait until put is done*/
	bool done = false;

	while (!done) {
	    PAMI_Context_advance (pami_transport->context, 100);

	    for (int i = 0; i < pami_transport->rput_done.size(); i++) {
		if (pami_transport->rput_done[i] == response.message_id) {
		    done = true;
		    pami_transport->rput_done.erase(pami_transport->rput_done.begin() + i);

		    break;
		}
	    }
	}

	/*Destroy the mem region*/
	result = PAMI_Memregion_destroy (pami_transport->context, &response.mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
    }

    return ret;
}
