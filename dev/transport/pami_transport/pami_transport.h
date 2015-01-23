#ifndef OPTIQ_PAMI_TRANSPORT
#define OPTIQ_PAMI_TRANSPORT

#include <pami.h>

struct optiq_pami_transport {
    int rank;
    int size;
    size_t num_contexts;
    pami_client_t client;
    pami_context_t context;
    pami_endpoint_t *endpoints;
};

extern "C" struct optiq_pami_transport *pami_transport;

void optiq_pami_transport_init();

struct optiq_pami_transport* optiq_get_pami_transport();

void optiq_pami_rput_done_fn(pami_context_t context, void *cookie, pami_result_t result);

void optiq_pami_rput_rdone_fn(pami_context_t context, void *cookie, pami_result_t result);

static void decrement (pami_context_t context, void *cookie, pami_result_t result);

int optiq_pami_rput(pami_client_t client, pami_context_t context, pami_memregion_t *local_mr, size_t local_offset, size_t nbytes, pami_endpoint_t &endpoint, pami_memregion_t *remote_mr, size_t remote_offset, void *cookie, pami_event_function rput_done_fn, void *rput_rdone_fn);

#endif
