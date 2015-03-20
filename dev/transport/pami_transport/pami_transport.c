#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <sys/time.h> 

#include "opi.h"

#include "pami_transport.h"

struct optiq_pami_transport *pami_transport = NULL;

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

    /*Receive memory request*/
    fn.p2p = optiq_recv_mem_request_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    OPTIQ_MEM_REQUEST,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Receive memory response - for general rput/send*/
    fn.p2p = optiq_recv_mem_response_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    OPTIQ_MEM_RESPONSE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Receive memory response - for general rput/send*/
    fn.p2p = optiq_recv_num_dests_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    BROADCAST_NUM_DESTS,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Receive memory response - for forwarding/destination request*/
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

    /*Path done notification*/
    fn.p2p = optiq_recv_path_done_notification_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    PATH_DONE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Job done notification*/
    fn.p2p = optiq_recv_job_done_notification_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    JOB_DONE,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Rput done notification*/
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

    fn.p2p = optiq_recv_mr_forward_request_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    MR_FORWARD_REQUEST,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    fn.p2p = optiq_recv_mr_destination_request_fn;
    result = PAMI_Dispatch_set (pami_transport->context,
	    MR_DESTINATION_REQUEST,
	    fn,
	    (void *) pami_transport,
	    options);

    assert(result == PAMI_SUCCESS);
    if (result != PAMI_SUCCESS) {
	return;
    }

    /*Other initialization*/
    pami_transport->transport_info.initialized = false;
    pami_transport->transport_info.finalized = false;

    if (!pami_transport->transport_info.initialized) {
	optiq_transport_info_init(pami_transport);
    }
}

void optiq_transport_info_init (struct optiq_pami_transport *pami_transport)
{
    if (pami_transport->transport_info.initialized) {
	return;
    }

    pami_transport->transport_info.initialized = true;

    int num_rput_cookies = OPTIQ_NUM_RPUT_COOKIES;
    int num_message_headers = OPTIQ_NUM_MESSAGE_HEADERS;

    /*Allocate memory for rput cookies*/
    pami_transport->transport_info.rput_cookies.clear();
    for (int i = 0; i < num_rput_cookies; i++)
    {
	struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)calloc(1, sizeof(struct optiq_rput_cookie));
	rput_cookie->pami_transport = pami_transport;
	pami_transport->transport_info.rput_cookies.push_back(rput_cookie);
    }

    /*Allocate memory for message headers*/
    pami_transport->transport_info.message_headers.clear();
    for (int i = 0; i < num_message_headers; i++)
    {
	struct optiq_message_header *message_header = (struct optiq_message_header *)calloc(1, sizeof(struct optiq_message_header));
	pami_transport->transport_info.message_headers.push_back(message_header);
    }

    /*Allocate and register forward memory*/
    int forward_buf_size = OPTIQ_FORWARD_BUFFER_SIZE;
    char *forward_buf = (char *) malloc (forward_buf_size);
    struct optiq_memregion *forward_mr = (struct optiq_memregion *) calloc (1, sizeof(struct optiq_memregion));

    size_t bytes;
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, forward_buf, forward_buf_size, &bytes, &forward_mr->mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < forward_buf_size) {
	printf("Registered less\n");
    }

    pami_transport->transport_info.forward_buf = forward_buf;
    pami_transport->transport_info.forward_mr = forward_mr;
    pami_transport->transport_info.forward_mr->offset = 0;

    pami_transport->transport_info.global_header_id = 0;
    pami_transport->transport_info.num_queues = 2;
    pami_transport->transport_info.current_queue = 0;

    pami_transport->transport_info.forward_headers.clear();
    pami_transport->transport_info.complete_rputs.clear();
    pami_transport->transport_info.send_headers.clear();
    pami_transport->transport_info.processing_headers.clear();
    pami_transport->transport_info.mr_responses.clear();
    pami_transport->transport_info.mem_responses.clear();
    pami_transport->transport_info.mem_requests.clear();
    pami_transport->transport_info.rput_done.clear();  

    pami_transport->transport_info.header_ids_map.clear();

    pami_transport->transport_info.num_requests = 0;

    pami_transport->transport_info.fwd_mem_req = fwd_mem_req_after_rput_done;
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
    /*timeval t0, t1, t2, t3;
      gettimeofday(&t0, NULL);*/

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

    /*gettimeofday(&t1, NULL);
      opi.sendimm_time += (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);*/

    return ret;
}

int optiq_pami_transport_send (void *send_buf, int send_bytes, int dest_rank)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    struct optiq_mem_request request;
    request.message_id = pami_transport->transport_info.global_header_id;
    pami_transport->transport_info.global_header_id = (pami_transport->transport_info.global_header_id++) % INT_MAX;
    request.length = send_bytes;
    request.source_id = pami_transport->rank;

    /*Request a memory region from sender side*/
    optiq_pami_send_immediate(pami_transport->context, OPTIQ_MEM_REQUEST, NULL, 0, &request, sizeof (struct optiq_mem_request), pami_transport->endpoints[dest_rank]);

    int ret = 0;

    size_t bytes;
    struct optiq_memregion send_mr;
    int send_offset = 0;

    /*Register its own memory*/
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, send_buf, send_bytes, &bytes, &send_mr.mr);
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

	for (int i = 0; i < pami_transport->transport_info.mem_responses.size(); i++) {
	    if (pami_transport->transport_info.mem_responses[i].message_id == request.message_id) {
		response = pami_transport->transport_info.mem_responses[i];
		pami_transport->transport_info.mem_responses.erase (pami_transport->transport_info.mem_responses.begin() + i);
		found = true;

		break;
	    }
	}
    }

    unsigned send_cookie = 1;

    /*Put data*/
    optiq_pami_rput(pami_transport->client, pami_transport->context, &send_mr.mr, send_offset, send_bytes, pami_transport->endpoints[dest_rank], &response.mr, response.offset, &send_cookie, NULL, optiq_pami_decrement);

    /*Wait until the put is done at remote side*/
    while (send_cookie == 1) {
	PAMI_Context_advance (pami_transport->context, 100);
    }


    /*Notify that the rput is done*/
    optiq_pami_send_immediate (pami_transport->context, OPTIQ_RPUT_DONE, NULL, NULL, &response.message_id, sizeof(int), pami_transport->endpoints[dest_rank]);

    /*Destroy the memregion was used*/
    result = PAMI_Memregion_destroy (pami_transport->context, &send_mr.mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    }

    return ret;
}

int optiq_pami_transport_recv (void *recv_buf, int recv_bytes, int source_rank)
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    struct optiq_mem_request request;
    bool found = false;

    while (!found) 
    {
	PAMI_Context_advance (pami_transport->context, 100);

	for (int i = 0; i < pami_transport->transport_info.mem_requests.size(); i++)
	{
	    if (pami_transport->transport_info.mem_requests[i].source_id == source_rank && pami_transport->transport_info.mem_requests[i].length == recv_bytes) 
	    {
		request = pami_transport->transport_info.mem_requests[i];
		found = true;
		break;
	    }
	}
    }

    int ret = 0;

    size_t bytes;

    /* Send the mem region back to the dest */
    struct optiq_mem_response response;
    response.dest_rank = pami_transport->rank;
    response.offset = 0;
    response.message_id = request.message_id;

    /* Register for its own mem region */
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, recv_buf, recv_bytes, &bytes, &response.mr);
    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } else if (bytes < recv_bytes) {
	printf("Registered less\n");
    }

    optiq_pami_send_immediate (pami_transport->context, OPTIQ_MEM_RESPONSE, NULL, NULL, &response, sizeof(struct optiq_mem_response), pami_transport->endpoints[source_rank]);

    /* Wait until put is done */
    bool done = false;

    while (!done) 
    {
	PAMI_Context_advance (pami_transport->context, 100);

	for (int i = 0; i < pami_transport->transport_info.rput_done.size(); i++) 
	{
	    if (pami_transport->transport_info.rput_done[i] == response.message_id) 
	    {
		done = true;
		pami_transport->transport_info.rput_done.erase(pami_transport->transport_info.rput_done.begin() + i);

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

	while (!found) 
	{
	    PAMI_Context_advance (pami_transport->context, 100);

	    for (int i = 0; i < pami_transport->transport_info.mem_responses.size(); i++) 
	    {
		if (pami_transport->transport_info.mem_responses[i].dest_rank == remote_rank)
		{
		    response = pami_transport->transport_info.mem_responses[i];
		    pami_transport->transport_info.mem_responses.erase (pami_transport->transport_info.mem_responses.begin() + i);
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
	optiq_pami_send_immediate (pami_transport->context, OPTIQ_RPUT_DONE, NULL, NULL, &response.message_id, sizeof(int), pami_transport->endpoints[remote_rank]);

	/*Destroy the memregion was used*/
	result = PAMI_Memregion_destroy (pami_transport->context, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
    }

    if (pami_transport->rank == remote_rank)
    {
	size_t bytes;

	/* Send the mem region back to the dest */
	struct optiq_mem_response response;
	response.dest_rank = pami_transport->rank;
	response.offset = 0;
	response.message_id = pami_transport->transport_info.global_header_id;
	pami_transport->transport_info.global_header_id = (pami_transport->transport_info.global_header_id++) % INT_MAX;

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

	    for (int i = 0; i < pami_transport->transport_info.rput_done.size(); i++) {
		if (pami_transport->transport_info.rput_done[i] == response.message_id) {
		    done = true;
		    pami_transport->transport_info.rput_done.erase(pami_transport->transport_info.rput_done.begin() + i);

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

void optiq_recv_mem_request_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->transport_info.mem_requests.push_back(*(struct optiq_mem_request *)data);
}

void optiq_recv_mem_response_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->transport_info.mem_responses.push_back(*(struct optiq_mem_response *)data);
}

void optiq_recv_num_dests_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->sched->all_num_dests[origin] = *((int *) data);
    pami_transport->sched->active_immsends--;
}

void optiq_recv_rput_done_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    int message_id = (*(int *)data);

    pami_transport->transport_info.rput_done.push_back(message_id);
}

void optiq_recv_mr_forward_request_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    struct optiq_message_header *mh = (struct optiq_message_header *) data;

    pami_transport->transport_info.forward_mr->header_id = mh->header_id;

    optiq_pami_send_immediate (pami_transport->context, MR_RESPONSE, NULL, 0, pami_transport->transport_info.forward_mr, sizeof(struct optiq_memregion), pami_transport->endpoints[origin]);

    /*printf("Rank %d sent a forward mem response to %d, offset = %d\n", pami_transport->rank, origin, pami_transport->transport_info.forward_mr->offset);*/
    pami_transport->transport_info.forward_mr->offset += mh->length;

    if (pami_transport->transport_info.forward_mr->offset >= OPTIQ_FORWARD_BUFFER_SIZE) {
	pami_transport->transport_info.forward_mr->offset = 0;
    }

    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = mh->header_id;
    stamp.eventtype = OPTIQ_EVENT_RECV_MEM_REQ;
    opi.timestamps.push_back(stamp);

    /* If the current rank wants to ask the next dest for mem region immediate */
    if (pami_transport->transport_info.fwd_mem_req == fwd_mem_req_imm)
    {
	/* Send another request for next dest */
	int new_header_id = pami_transport->transport_info.global_header_id;
	pami_transport->transport_info.global_header_id++;

	std::pair<int, int> ids = std::make_pair (mh->header_id, mh->path_id);
	std::pair<std::pair<int, int>, int> oldnewids = std::make_pair (ids, new_header_id);
	pami_transport->transport_info.header_ids_map.push_back(oldnewids);

	mh->header_id = new_header_id;

	optiq_pami_transport_mem_request(mh);
    }
    else if (pami_transport->transport_info.fwd_mem_req == fwd_mem_req_queue)
    {
	struct optiq_message_header *message_header = pami_transport->transport_info.message_headers.back();
	pami_transport->transport_info.message_headers.pop_back();
	memcpy (message_header, mh, sizeof(struct optiq_message_header));

	pami_transport->transport_info.forward_headers.push_back(message_header);
    }
}

void optiq_recv_mr_destination_request_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    int source = (*(int *)data);

    pami_transport->sched->recv_mr.header_id = *((int*)header);
    pami_transport->sched->recv_mr.offset = pami_transport->sched->rdispls[source];

    optiq_pami_send_immediate (pami_transport->context, MR_RESPONSE, NULL, 0, &pami_transport->sched->recv_mr, sizeof(struct optiq_memregion), pami_transport->endpoints[origin]);

    /*printf("Rank %d sent a destination mem response to %d, offset = %d\n", pami_transport->rank, origin, pami_transport->sched->recv_mr.offset);*/

    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = *((int*)header);
    stamp.eventtype = OPTIQ_EVENT_RECV_MEM_REQ;
    opi.timestamps.push_back(stamp);
}

void optiq_recv_rput_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    /*If the message is not for itself*/
    struct optiq_message_header *message_header = pami_transport->transport_info.message_headers.back();
    pami_transport->transport_info.message_headers.pop_back();

    memcpy(message_header, data, data_size);

    if(pami_transport->rank == message_header->dest) {
	pami_transport->sched->expecting_length -= message_header->length;
	/*printf("Rank %d get a put done notification from %d with data size %d, expecting_length = %d\n", pami_transport->rank, origin, message_header->length, pami_transport->sched->expecting_length);*/
	/*printf("Rank %d received data from %d size = %d\n", pami_transport->rank, message_header->source, message_header->length);*/
	pami_transport->sched->recv_bytes[message_header->source] += message_header->length;
    }
    else 
    {
	if (pami_transport->transport_info.fwd_mem_req == fwd_mem_req_after_rput_done)
	{
	    pami_transport->transport_info.forward_headers.push_back(message_header);
	}
	else 
	{
	    /* Assign new header id and put into processing queue*/
	    for (int i = 0; i < pami_transport->transport_info.header_ids_map.size(); i++)
	    {
		if (pami_transport->transport_info.header_ids_map[i].first.first == message_header->header_id &&
		        pami_transport->transport_info.header_ids_map[i].first.second == message_header->path_id)
		{
		    message_header->header_id = pami_transport->transport_info.header_ids_map[i].second;
		}
	    }
	    pami_transport->transport_info.processing_headers.push_back(message_header);
	}
    }

    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = message_header->header_id;
    stamp.eventtype = OPTIQ_EVENT_RECV_RPUT_DONE;
    opi.timestamps.push_back(stamp);
    
    /*printf("Rank %d get a put done notification from %d with data size %d\n", pami_transport->rank, origin, message_header->length);*/
}

void optiq_recv_mr_response_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    struct optiq_memregion *mr = (struct optiq_memregion *) data;
    pami_transport->transport_info.mr_responses.push_back(*mr);

    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = mr->header_id;
    stamp.eventtype = OPTIQ_EVENT_RECV_MEM_RES;
    opi.timestamps.push_back(stamp);

    pami_transport->transport_info.num_requests--;
    
    /*printf("Rank %d recv a response from %d, offset = %d\n", pami_transport->rank, origin, mr->offset);*/
}

void optiq_recv_path_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;
    int path_id = *( (int *) header );
    pami_transport->sched->num_active_paths--;

    for (int i = 0; i < pami_transport->sched->intermediate_notify_list.size(); i++) 
    {
	if (pami_transport->sched->intermediate_notify_list[i].first == path_id)
	{
	    for (int j = 0; j < pami_transport->sched->intermediate_notify_list[i].second.size(); j++)
	    {
		int dest = pami_transport->sched->intermediate_notify_list[i].second[j];
		optiq_pami_send_immediate(pami_transport->context, PATH_DONE, &path_id, sizeof(int), NULL, 0, pami_transport->endpoints[dest]);
	    }
	    pami_transport->sched->intermediate_notify_list.erase(pami_transport->sched->intermediate_notify_list.begin() + i);
	}
    }

    /*printf("Rank %d recv path done notification from %d\n", pami_transport->rank, origin);*/
}

void optiq_recv_job_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->sched->remaining_jobs--;
}

void optiq_pami_rput_rdone_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)cookie;
    struct optiq_pami_transport *pami_transport = rput_cookie->pami_transport;

    pami_transport->transport_info.complete_rputs.push_back(rput_cookie);
    pami_transport->sched->sent_bytes += rput_cookie->message_header->length;

    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = rput_cookie->message_header->header_id;
    stamp.eventtype = OPTIQ_EVENT_RPUT_RDONE;
    opi.timestamps.push_back(stamp);
}

void optiq_pami_transport_mem_request(struct optiq_message_header *header)
{
    timeval t2, t3;

    gettimeofday(&t2, NULL);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    /*Notify the size, ask for mem region*/
    int dest = pami_transport->sched->next_dests[header->path_id];
    if (dest == -1) {
        printf("Rank %d received invalid message with path_id = %d, [s %d d %d], size = %d\n", pami_transport->rank, header->path_id, header->source, header->dest, header->length);
    }

    /*If the next destination is final destination*/
    timeval tx;
    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = header->header_id;
    stamp.eventtype = OPTIQ_EVENT_MEM_REQ;
    opi.timestamps.push_back(stamp);

    /*printf("Rank %d req mem from %d  with path_id = %d, [s %d d %d], size = %d\n", pami_transport->rank, dest, header->path_id, header->source, header->dest, header->length);*/

    if (dest == header->dest)
    {
	/*printf("header_id = %d dest = %d\n", header->header_id, dest);*/
        optiq_pami_send_immediate(pami_transport->context, MR_DESTINATION_REQUEST, &header->header_id, sizeof(int), &header->source, sizeof(int), pami_transport->endpoints[dest]);
    }
    else
    {
        optiq_pami_send_immediate(pami_transport->context, MR_FORWARD_REQUEST, NULL, 0, header, sizeof(struct optiq_message_header), pami_transport->endpoints[dest]);
    }

    pami_transport->transport_info.num_requests++;

    gettimeofday(&t3, NULL);
    opi.total_mem_req_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
}

void optiq_pami_transport_get_message ()
{
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();
    bool fwd = false;

    if (pami_transport->transport_info.send_headers.size() + pami_transport->transport_info.forward_headers.size() > 0)
    {
	std::vector<struct optiq_message_header *> *mh;
	struct optiq_message_header *header = NULL;

	if (schedule->dmode == DQUEUE_LOCAL_MESSAGE_FIRST) 
	{
	    if (pami_transport->transport_info.send_headers.size() > 0) {
		mh = &(pami_transport->transport_info.send_headers);
	    } else {
		mh = &(pami_transport->transport_info.forward_headers);
		fwd = true;
	    }
	} 
	else if (schedule->dmode == DQUEUE_FORWARD_MESSAGE_FIRST) 
	{
	    if (pami_transport->transport_info.forward_headers.size() > 0) {
                mh = &(pami_transport->transport_info.forward_headers);
		fwd = true;
            } else {
                mh = &(pami_transport->transport_info.send_headers);
            }
	}
	else if (schedule->dmode == DQUEUE_ROUND_ROBIN)
	{
	    if (pami_transport->transport_info.current_queue == 0) 
	    {
		if (pami_transport->transport_info.send_headers.size() > 0) 
		{
		    mh = &(pami_transport->transport_info.send_headers);
		    pami_transport->transport_info.current_queue = 1;
		} else {
		    mh = &(pami_transport->transport_info.forward_headers);
		    fwd = true;
		}
	    }
	    else if (pami_transport->transport_info.current_queue == 1) 
            {
		if (pami_transport->transport_info.forward_headers.size() > 0) 
		{
		    mh = &(pami_transport->transport_info.forward_headers);
		    pami_transport->transport_info.current_queue = 0;
		    fwd = true;
		} else {
		    mh = &(pami_transport->transport_info.send_headers);
		}
	    }
	}

	/* Get the header out of the queue */
	header = mh->front();
	mh->erase (mh->begin());

	/* Assign header value */
	int new_header_id = pami_transport->transport_info.global_header_id;
	pami_transport->transport_info.global_header_id++;

	if (fwd && pami_transport->transport_info.fwd_mem_req == fwd_mem_req_queue)
	{
	    std::pair<int, int> ids = std::make_pair (header->header_id, header->path_id);
	    std::pair<std::pair<int, int>, int> oldnewids = std::make_pair (ids, new_header_id);
	    pami_transport->transport_info.header_ids_map.push_back(oldnewids);

	    header->header_id = new_header_id;
	}
	else
	{
	    header->header_id = new_header_id;
	    pami_transport->transport_info.processing_headers.push_back(header);
	}

	/* Request memory from next dest */
	optiq_pami_transport_mem_request(header);

	if (fwd  && pami_transport->transport_info.fwd_mem_req == fwd_mem_req_queue) {
	    pami_transport->transport_info.message_headers.push_back(header);
	}
    }
}

void optiq_pami_transport_send_local_mem_requests ()
{
    timeval t2, t3;
    gettimeofday(&t2, NULL);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    for (int i = 0; i < pami_transport->transport_info.send_headers.size(); i++)
    {
        struct optiq_message_header *header = pami_transport->transport_info.send_headers[i];

	header->header_id = pami_transport->transport_info.global_header_id;
        pami_transport->transport_info.global_header_id++;
        pami_transport->transport_info.processing_headers.push_back(header);

	optiq_pami_transport_mem_request(header);
    }

    pami_transport->transport_info.send_headers.clear();

    gettimeofday(&t3, NULL);
    opi.local_mem_req_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
}

void optiq_pami_transport_execute(struct optiq_pami_transport *pami_transport)
{
    timeval t0, t1, t2, t3, tx;
    gettimeofday(&t0, NULL);

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    gettimeofday(&tx, NULL);
    struct timestamp stamp;
    stamp.tv = tx;
    stamp.eventid = 0;
    stamp.eventtype = OPTIQ_EVENT_START;
    opi.timestamps.push_back(stamp);

    //optiq_pami_transport_send_local_mem_requests();

    while (pami_transport->sched->num_active_paths > 0)
    {
	gettimeofday(&t2, NULL);

	PAMI_Context_advance(pami_transport->context, 100);

	gettimeofday(&t3, NULL);
        opi.context_advance_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);

	/*If all jobs are done*/
	if (pami_transport->sched->num_active_paths == 0) {
	    break;
	}

	/*If a destination has received all of its data*/
	if (pami_transport->sched->isDest && pami_transport->sched->expecting_length == 0) 
	{
	    gettimeofday(&t2, NULL);

	    int i = 0;
	    while (pami_transport->sched->notify_list.size() > 0) 
	    {
		i = i % pami_transport->sched->notify_list.size();

		while (pami_transport->sched->notify_list[i].second.size() > 0)
		{
		    int path_id = pami_transport->sched->notify_list[i].first;
		    int dest = pami_transport->sched->notify_list[i].second.back();
		    pami_transport->sched->notify_list[i].second.pop_back();

		    if (dest != rank) {
			optiq_pami_send_immediate(pami_transport->context, PATH_DONE, &path_id, sizeof(int), NULL, 0, pami_transport->endpoints[dest]);
		    }

		    break;
		}

		if (pami_transport->sched->notify_list[i].second.size() == 0) 
		{
		    pami_transport->sched->notify_list.erase(pami_transport->sched->notify_list.begin() + i);

		    pami_transport->sched->num_active_paths--;
		}

		i++;
	    }

	    pami_transport->sched->expecting_length = -1;
	    gettimeofday(&t3, NULL);
	    opi.notification_done_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
	}

	/*if (true) {
	    printf("Rank %d local size = %d, forward size = %d, num_active_paths = %d, isDest = %d, expecting_length = %d, processing_headers = %d, mr_responses = %d\n", pami_transport->rank, pami_transport->transport_info.send_headers.size(), pami_transport->transport_info.forward_headers.size(), pami_transport->sched->num_active_paths, pami_transport->sched->isDest, pami_transport->sched->expecting_length, pami_transport->transport_info.processing_headers.size(), pami_transport->transport_info.mr_responses.size());
	}*/

	/*If there is a mem region ready to be transferred*/
	bool matched = true;

	while (matched) 
	{
	    matched = false;

	    gettimeofday(&t2, NULL);

	    /*Search for the message with the same header_id*/
            struct optiq_message_header *header = NULL;
	    struct optiq_memregion far_mr;

	    for (int r = 0; r < pami_transport->transport_info.mr_responses.size(); r++)
	    {
		far_mr = pami_transport->transport_info.mr_responses[r];

		for (int i = 0; i < pami_transport->transport_info.processing_headers.size(); i++)
		{
		    /*printf("Rank %d processing header id =  %d, far mr header id = %d\n", rank, pami_transport->transport_info.processing_headers[i]->header_id, far_mr.header_id);*/
		    if (pami_transport->transport_info.processing_headers[i]->header_id == far_mr.header_id)
		    {
			header = pami_transport->transport_info.processing_headers[i];
			pami_transport->transport_info.processing_headers.erase(pami_transport->transport_info.processing_headers.begin() + i);
			matched = true;
			break;
		    }
		}

		if (matched) 
		{
		    pami_transport->transport_info.mr_responses.erase(pami_transport->transport_info.mr_responses.begin() + r);
		    break;
		}
	    }

	    gettimeofday(&t3, NULL);
	    opi.matching_procesing_header_mr_response_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);

	    gettimeofday(&t2, NULL);
	    if (matched)
	    {
		/*Actual rput data*/
		struct optiq_rput_cookie *rput_cookie = pami_transport->transport_info.rput_cookies.back();
		pami_transport->transport_info.rput_cookies.pop_back();

		int dest = pami_transport->sched->next_dests[header->path_id];
		rput_cookie->message_header = header;
		rput_cookie->dest = dest;

		/*Rput for destination message. This is because if the next dest is final dest, the dest only gives the offset of the source, not offset for each chunk*/
		if (dest == header->dest)
		{
		    far_mr.offset += header->original_offset;
		}

		/*if (true) {
		    printf("Rank %d rput %d bytes of orin[s %d, d %d] along path_id = %d of data to %d\n", pami_transport->rank, header->length, header->source, header->dest, header->path_id, dest);
		}*/

	        gettimeofday(&tx, NULL);
		struct timestamp stamp;
	        stamp.tv = tx;
	        stamp.eventid = header->header_id;
		stamp.eventtype = OPTIQ_EVENT_RPUT;
		opi.timestamps.push_back(stamp);

		optiq_pami_rput(pami_transport->client, pami_transport->context, &header->mem.mr, header->mem.offset, header->length, pami_transport->endpoints[dest], &far_mr.mr, far_mr.offset, rput_cookie, NULL, optiq_pami_rput_rdone_fn);

		/*Now the header will contain the far memregion instead of local memregion*/
		memcpy(&header->mem, &far_mr, sizeof(struct optiq_memregion));
	    }
	    gettimeofday(&t3, NULL);
	    opi.post_rput_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
	}

	/*If a put is done, notify the remote destination*/
	gettimeofday(&t2, NULL);
	if (pami_transport->transport_info.complete_rputs.size() > 0)
	{
	    struct optiq_rput_cookie *complete_rput = pami_transport->transport_info.complete_rputs.front();
	    pami_transport->transport_info.complete_rputs.erase(pami_transport->transport_info.complete_rputs.begin());

	    struct optiq_message_header *complete_header = complete_rput->message_header;

	    /*Notify that rput is done*/
	    gettimeofday(&tx, NULL);
            struct timestamp stamp;
            stamp.tv = tx;
            stamp.eventid = complete_header->header_id;
            stamp.eventtype = OPTIQ_EVENT_RPUT_DONE_NOTIFY;
            opi.timestamps.push_back(stamp);

	    optiq_pami_send_immediate(pami_transport->context, RPUT_DONE, NULL, 0, complete_header, sizeof(struct optiq_message_header), pami_transport->endpoints[complete_rput->dest]);

	    pami_transport->transport_info.message_headers.push_back(complete_header);
	    pami_transport->transport_info.rput_cookies.push_back(complete_rput);
	}
	gettimeofday(&t3, NULL);
	opi.check_complete_rput_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);

	/*If there is a request to send a message*/
        gettimeofday(&t2, NULL);

	if (pami_transport->transport_info.num_requests < 3) {
	    optiq_pami_transport_get_message();
	}

	gettimeofday(&t3, NULL);
        opi.get_header_time += (t3.tv_sec - t2.tv_sec) * 1e6 + (t3.tv_usec - t2.tv_usec);
    }

    gettimeofday(&t1, NULL);

    opi.transfer_time = (t1.tv_sec - t0.tv_sec) * 1e6 + (t1.tv_usec - t0.tv_usec);
    opi.recv_len = pami_transport->sched->recv_len;
}

void optiq_pami_transport_info_status(struct optiq_transport_info &transport_info, int rank)
{
    printf("Rank = %d, forward_mr->offset = %d\n", rank, transport_info.forward_mr->offset);

    printf("Rank = %d, rput_cookies.size() = %d\n", rank, transport_info.rput_cookies.size());
    printf("Rank = %d, complete_rputs.size() = %d\n", rank, transport_info.complete_rputs.size());

    printf("Rank = %d, forward_headers.size() = %d\n", rank, transport_info.forward_headers.size());
    printf("Rank = %d, message_headers.size() = %d\n", rank, transport_info.message_headers.size());
    printf("Rank = %d, send_headers.size() = %d\n", rank, transport_info.send_headers.size());
    printf("Rank = %d, processing_headers.size() = %d\n", rank, transport_info.processing_headers.size());

    printf("Rank = %d, mr_responses.size() = %d\n", rank, transport_info.mr_responses.size());
    printf("Rank = %d, global_header_id = %d\n", rank, transport_info.global_header_id);
}

void optiq_pami_transport_sched_status(struct optiq_schedule *sched, int rank)
{
    printf("Rank = %d, Remain job = %d\n", rank, sched->remaining_jobs);
    printf("Rank = %d, active_paths = %d\n", rank, sched->num_active_paths);
    printf("Rank = %d, expecting_length = %d\n", rank, sched->expecting_length);
}

void optiq_pami_transport_status(struct optiq_pami_transport *pami_transport)
{
    optiq_pami_transport_info_status(pami_transport->transport_info, pami_transport->rank);
    optiq_pami_transport_sched_status(pami_transport->sched, pami_transport->rank);
}

void optiq_transport_info_finalize(struct optiq_pami_transport *pami_transport)
{
    if (pami_transport->transport_info.finalized) {
	return;
    }

    pami_transport->transport_info.finalized = true;

    /*Destroy memregion*/
    pami_result_t result = PAMI_Memregion_destroy (pami_transport->context, &pami_transport->transport_info.forward_mr->mr);

    if (result != PAMI_SUCCESS) {
	printf("No success\n");
    } 

    /*Free memory*/
    free(pami_transport->transport_info.forward_buf);

    for (int i = 0; i < pami_transport->transport_info.rput_cookies.size(); i++)
    {
	free(pami_transport->transport_info.rput_cookies[i]);
    }

    for (int i = 0; i < pami_transport->transport_info.message_headers.size(); i++)
    {
	free(pami_transport->transport_info.message_headers[i]);
    }
}

int optiq_pami_transport_finalize()
{
    struct optiq_pami_transport* pami_transport = optiq_pami_transport_get();

    if (!pami_transport->transport_info.finalized) {
	optiq_transport_info_finalize(pami_transport);
    }

    /* Free memory */
    free(pami_transport->endpoints);

    /* Destroy context */
    pami_result_t result;
    result = PAMI_Context_destroyv(&pami_transport->context, pami_transport->num_contexts);
    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
	return 1;
    }

    /* Destroy client */
    result = PAMI_Client_destroy(&pami_transport->client);
    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
	return 1;
    }

    /* Free the pami_transport itself*/
    free(pami_transport);

    return 0;
}

void optiq_pami_transport_print()
{
    printf("Rank %d size %d\n", pami_transport->rank, pami_transport->size);
}
