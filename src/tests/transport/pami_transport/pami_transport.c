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

void optiq_pami_init_extra(struct optiq_pami_transport *pami_transport)
{
    int num_rput_cookies = OPTIQ_NUM_RPUT_COOKIES;
    int num_message_headers = OPTIQ_NUM_MESSAGE_HEADERS;

    /*Allocate memory for rput cookies*/
    for (int i = 0; i < num_rput_cookies; i++) {
        struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)calloc(1, sizeof(struct optiq_rput_cookie));
        rput_cookie->pami_transport = pami_transport;
        pami_transport->extra.rput_cookies.push_back(rput_cookie);
    }

    /*Allocate memory for message headers*/
    for (int i = 0; i < num_message_headers; i++) {
        struct optiq_message_header *message_header = (struct optiq_message_header *)calloc(1, sizeof(struct optiq_message_header));
        pami_transport->extra.message_headers.push_back(message_header);
    }

    /*Allocate and register forward memory*/
    int forward_buf_size = OPTIQ_FORWARD_BUFFER_SIZE;
    char *forward_buf = (char *) malloc(forward_buf_size);
    struct optiq_memregion *forward_mr = (struct optiq_memregion *) malloc (sizeof(struct optiq_memregion));

    size_t bytes;
    pami_result_t result = PAMI_Memregion_create (pami_transport->context, forward_buf, forward_buf_size, &bytes, &forward_mr->mr);

    if (result != PAMI_SUCCESS) {
        printf("No success\n");
    } else if (bytes < forward_buf_size) {
        printf("Registered less\n");
    }

    pami_transport->extra.forward_mr = forward_mr;
    pami_transport->extra.forward_mr->offset = 0;
}

void optiq_execute_jobs(struct optiq_pami_transport *pami_transport)
{
    while (pami_transport->bulk.remaining_jobs > 0) 
    {
	PAMI_Context_advance(pami_transport->context, 100);

	/*If all jobs are done*/
	if (pami_transport->bulk.remaining_jobs == 0) {
            break;
        }

	/*If a destination has received all of its data*/
	if (pami_transport->bulk.isDest && pami_transport->bulk.expecting_length == 0) {
            for (int i = 0; i < pami_transport->size; i++) {
                optiq_pami_send_immediate(pami_transport->context, JOB_DONE, NULL, 0, NULL, 0, pami_transport->endpoints[i]);
            }
            pami_transport->bulk.expecting_length = -1;
        }

	/*If there is a request to send a message*/
	if (pami_transport->extra.send_headers.size() + pami_transport->extra.forward_headers.size() > 0)
	{
	    struct optiq_message_header *header = NULL;
	    if (pami_transport->extra.send_headers.size() > 0) 
	    {
		header = pami_transport->extra.send_headers.front();
		pami_transport->extra.send_headers.erase(pami_transport->extra.send_headers.begin());
	    }
	    else if (pami_transport->extra.forward_headers.size() > 0) 
	    {
		header = pami_transport->extra.forward_headers.front();
		pami_transport->extra.forward_headers.erase(pami_transport->extra.forward_headers.begin());
	    }

	    header->header_id = pami_transport->extra.global_header_id;
	    pami_transport->extra.global_header_id++;
	    pami_transport->extra.processing_headers.push_back(header);

	    /*Notify the size, ask for mem region*/
            int dest = pami_transport->bulk.next_dest[header->path_id];

	    /*If the next destination is final destination*/
	    if (dest == header->dest) {
		optiq_pami_send_immediate(pami_transport->context, MR_DESTINATION_REQUEST, &header->header_id, sizeof(int), &header->source, sizeof(int), pami_transport->endpoints[dest]);
	    } else {
		optiq_pami_send_immediate(pami_transport->context, MR_FORWARD_REQUEST, &header->header_id, sizeof(int), &header->length, sizeof(int), pami_transport->endpoints[dest]);
	    }
	}
	
	/*If there is a mem region ready to be transferred*/
	if (pami_transport->extra.mr_responses.size() > 0) 
	{
	    struct optiq_memregion far_mr = pami_transport->extra.mr_responses.front();
	    pami_transport->extra.mr_responses.erase(pami_transport->extra.mr_responses.begin());

	    /*Search for the message with the same header_id*/
	    struct optiq_message_header *header = NULL;
	    for (int i = 0; i < pami_transport->extra.processing_headers.size(); i++) 
	    {
		if (pami_transport->extra.processing_headers[i]->header_id == far_mr.header_id) 
		{
		    header = pami_transport->extra.processing_headers[i];
		    pami_transport->extra.processing_headers.erase(pami_transport->extra.processing_headers.begin() + i);
		    break;
		}
	    }
	    
	    /*Actual rput data*/
	    struct optiq_rput_cookie *rput_cookie = pami_transport->extra.rput_cookies.back();
            pami_transport->extra.rput_cookies.pop_back();

	    int dest = pami_transport->bulk.next_dest[header->path_id];
	    rput_cookie->message_header = header;
	    rput_cookie->dest = dest;


	    /*Rput for destination message. This is because if the next dest is final dest, the dest only gives the offset of the source, not offset for each chunk*/
	    if (dest == header->dest) {
		far_mr.offset += header->original_offset;
	    }

	    optiq_pami_rput(pami_transport->client, pami_transport->context, &header->mem.mr, header->mem.offset, header->length, pami_transport->endpoints[dest], &far_mr.mr, far_mr.offset, rput_cookie);

	    /*Now the header will contain the far memregion instead of local memregion*/
	    memcpy(&header->mem, &far_mr, sizeof(struct optiq_memregion));
	}

	/*If a put is done, notify the remote destination*/
	if (pami_transport->extra.complete_rputs.size() > 0) 
	{
	    struct optiq_rput_cookie *complete_rput = pami_transport->extra.complete_rputs.front();
	    pami_transport->extra.complete_rputs.erase(pami_transport->extra.complete_rputs.begin());

	    struct optiq_message_header *complete_header = complete_rput->message_header;

	    /*Notify that rput is done*/
	    optiq_pami_send_immediate(pami_transport->context, RPUT_DONE, NULL, 0, complete_header, sizeof(struct optiq_message_header), pami_transport->endpoints[complete_rput->dest]);

	    pami_transport->extra.message_headers.push_back(complete_header);
	    pami_transport->extra.rput_cookies.push_back(complete_rput);
	}
    }
}

void optiq_pami_rput_done_fn(pami_context_t context, void *cookie, pami_result_t result)
{
}

void optiq_pami_rput_rdone_fn(pami_context_t context, void *cookie, pami_result_t result)
{   
    struct optiq_rput_cookie *rput_cookie = (struct optiq_rput_cookie *)cookie;
    struct optiq_pami_transport *pami_transport = rput_cookie->pami_transport;

    pami_transport->extra.complete_rputs.push_back(rput_cookie);
    pami_transport->bulk.sent_bytes += rput_cookie->message_header->length;
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

void optiq_recv_job_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->bulk.remaining_jobs--;
}

void optiq_recv_rput_done_notification_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    /*If the message is not for itself*/
    struct optiq_message_header *message_header = pami_transport->extra.message_headers.back();
    pami_transport->extra.message_headers.pop_back();

    memcpy(message_header, data, data_size);

    if(pami_transport->rank == message_header->dest) {
	pami_transport->bulk.expecting_length -= message_header->length;
	//printf("Rank %d get a put done notification from %d with data size %d, expecting_length = %d\n", pami_transport->rank, origin, message_header->length, pami_transport->extra.expecting_length);
	//printf("Rank %d received data from %d size = %d\n", pami_transport->rank, message_header->source, message_header->length);
	pami_transport->bulk.recv_bytes[message_header->source] += message_header->length;
    } else {
	pami_transport->extra.forward_headers.push_back(message_header);
    }

    //printf("Rank %d get a put done notification from %d with data size %d\n", pami_transport->rank, origin, message_header->length);
}

void optiq_recv_mr_response_fn(pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->extra.mr_responses.push_back(*(struct optiq_memregion *)data);

    //printf("Rank %d recv a response from %d, offset = %d\n", pami_transport->rank, origin, ((struct optiq_memregion *)data)->offset);
}

void optiq_recv_mr_forward_request_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    pami_transport->extra.forward_mr->header_id = *((int*)header);

    optiq_pami_send_immediate (pami_transport->context, MR_RESPONSE, NULL, 0, pami_transport->extra.forward_mr, sizeof(struct optiq_memregion), pami_transport->endpoints[origin]);

    //printf("Rank %d sent a forward mem response to %d, offset = %d\n", pami_transport->rank, origin, pami_transport->extra.forward_mr->offset);

    pami_transport->extra.forward_mr->offset += (*(int *)data);

    if (pami_transport->extra.forward_mr->offset >= OPTIQ_FORWARD_BUFFER_SIZE) {
	pami_transport->extra.forward_mr->offset = 0;
    }
}

void optiq_recv_mr_destination_request_fn (pami_context_t context, void *cookie, const void *header, size_t header_size, const void *data, size_t data_size, pami_endpoint_t origin, pami_recv_t *recv)
{
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)cookie;

    int source = (*(int *)data);
    int offset = pami_transport->bulk.rdispls[source];

    pami_transport->bulk.recv_mr.header_id = *((int*)header);
    pami_transport->bulk.recv_mr.offset = offset;

    optiq_pami_send_immediate (pami_transport->context, MR_RESPONSE, NULL, 0, &pami_transport->bulk.recv_mr, sizeof(struct optiq_memregion), pami_transport->endpoints[origin]);

    //printf("Rank %d sent a destination mem response to %d, offset = %d\n", pami_transport->rank, origin, pami_transport->extra.recv_mr->offset);
}
