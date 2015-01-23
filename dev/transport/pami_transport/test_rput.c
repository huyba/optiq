#include <stdio.h>
#include <limits.h>
#include <string.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    optiq_pami_transport_init();

    struct optiq_pami_transport* pami_transport = optiq_get_pami_transport();

    int send_rank = 0;
    int send_bytes = 1024 * 1024;
    void *send_buf = malloc (send_bytes);

    for (int i = 0; i < send_bytes; i++) {
	((char*)send_buf)[i] = i % 128;
    }

    int recv_rank = 1;
    int recv_bytes = 1024 * 1024;
    void *recv_buf = malloc (recv_bytes);

    if (pami_transport->rank == send_rank) 
    {
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
		if (pami_transport->mem_responses[i].dest_rank == recv_rank) {
		    response = pami_transport->mem_responses[i];
		    pami_transport->mem_responses.erase (pami_transport->mem_responses.begin() + i);
		    found = true;

		    break;
		}
	    }
	}

	unsigned send_cookie = 1;

	/*Put data*/
	optiq_pami_rput(pami_transport->client, pami_transport->context, &send_mr, send_offset, send_bytes, pami_transport->endpoints[recv_rank], &response.mr, response.offset, &send_cookie, NULL, (void *) decrement);

	/*Wait until the put is done at remote side*/
	while (send_cookie == 1) {
	    PAMI_Context_advance (pami_transport->context, 100);
	}

	/*Notify that the rput is done*/
	optiq_pami_send_immediate (pami_transport->context, OPTIQ_RPUT_DONE, NULL, NULL, &response.message_id, sizeof(int), pami_transport->
endpoints[recv_rank]);

	/*Destroy the memregion was used*/
	result = PAMI_Memregion_destroy (pami_transport->context, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
    }

    if (pami_transport->rank == recv_rank)
    {
	size_t bytes;

	/*Send the mem region back to the dest*/
        struct optiq_mem_response response;
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

	optiq_pami_send_immediate (pami_transport->context, OPTIQ_MEM_RESPONSE, NULL, NULL, &response, sizeof(struct optiq_mem_response), pami_transport->endpoints[send_rank]);

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

    if (pami_transport->rank == recv_rank)
    {
	if (memcmp(send_buf, recv_buf, send_bytes) != 0) {
	    printf("Invalid data recv\n");
	} else {
	    printf("Valid data recv\n");
	}
    }

    return 0;
}
