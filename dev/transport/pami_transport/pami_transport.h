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

void pami_transport_init();

struct optiq_pami_transport* get_pami_transport();

#endif
