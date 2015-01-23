#include <stdlib.h>
#include <assert.h>

#include "pami_transport.h"

struct optiq_pami_transport *pami_transport;

void pami_transport_init()
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
}

struct optiq_pami_transport* get_pami_transport()
{
    return pami_transport;
}
