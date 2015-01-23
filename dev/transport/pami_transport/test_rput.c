#include <stdio.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    pami_transport_init();

    struct optiq_pami_transport* pami_transport = get_pami_transport();

    int send_rank = 0;
    int recv_rank = 1;

    int send_rank = 0;
    int send_bytes = 1024 * 1024;
    void *send_buf = malloc (send_bytes);

    int recv_rank = 1;
    int recv_bytes = 1024 * 1024;
    void recv_buf = malloc (recv_bytes);

    if (pami_transport->rank == send_rank) 
    {
	int bytes;
	pami_memregion_t send_mr;
	int local_offset = 0;

	/*Ask for the memregion from the recv side*/

	/*Register its own memory*/
        pami_result_t result = PAMI_Memregion_create (pami_transport->context, send_buf, send_bytes, &bytes, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < nbytes) {
	    printf("Registered less\n");
	}

	/*Wait until the mem region request is responded*/


	/*Put data*/
	optiq_pami_rput(pami_transport->client, pami_transport->context, &send_mr, send_offset, send_bytes, pami_transport->endpoints[recv_rank], remote_mr, size_t remote_offset, void *cookie, pami_event_function rput_done_fn, pami_event_function rput_rdone_fn);

	/*Wait until the put is done*/

	/*Destroy the memregion was used*/
	result = PAMI_Memregion_destroy (pami_transport->context, &send_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	}
    }

    if (pami_transport->rank == recv_rank)
    {
	int bytes;
	int recv_bytes = send_bytes;
	void recv_buf = malloc (recv_bytes);
	pami_memregion_t recv_mr;
	int remote_offset = 0;
	unsigned recv_cookie = 1;

	/*Register for its own mem region*/
	pami_result_t result = PAMI_Memregion_create (pami_transport->context, recv_buf, recv_bytes, &bytes, &recv_mr);
    	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < nbytes) {
	    printf("Registered less\n");
	}

	/*Wait for mem region request*/


	/*Response to the mem region request*/


	/*Wait until put is done*/


	/*Destroy the mem region*/
	result = PAMI_Memregion_destroy (pami_transport->context, &remote_mr);
	if (result != PAMI_SUCCESS) {
	    printf("No success\n");
	} else if (bytes < forward_buf_size) {
	    printf("Registered less\n");
	}
    }

    return 0;
}
