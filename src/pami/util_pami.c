#include <util_pami.h>

int pami_init(pami_client_t        * client,          /* in/out:  client      */
        pami_context_t       * context,         /* in/out:  context     */
        char                 * clientname,      /* in/out:  clientname  */
        size_t               * num_contexts,    /* in/out:  num_contexts*/
        pami_configuration_t * configuration,   /* in:      config      */
        size_t                 num_config,      /* in:      num configs */
        pami_task_t          * task_id,         /* out:     task id     */
        size_t               * num_tasks)       /* out:     num tasks   */
{
    pami_result_t        result        = PAMI_ERROR;
    char                 cl_string[]   = "PAMI_MULTIPATH";
    pami_configuration_t l_configuration;
    size_t               max_contexts;



    if(clientname == NULL)
        clientname = cl_string;

    /* Docs01:  Create the client */
    result = PAMI_Client_create (clientname, client, NULL, 0);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to initialize pami client %s: result = %d\n",
                clientname,result);
        return 1;
    }

    l_configuration.name = PAMI_CLIENT_NUM_CONTEXTS;
    result = PAMI_Client_query(*client, &l_configuration,1);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                l_configuration.name, result);
        return 1;
    }
    max_contexts = l_configuration.value.intval;
    *num_tasks = (*num_tasks<max_contexts)?*num_tasks:max_contexts;

    l_configuration.name = PAMI_CLIENT_TASK_ID;
    result = PAMI_Client_query(*client, &l_configuration,1);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                l_configuration.name, result);
        return 1;
    }
    *task_id = l_configuration.value.intval;

    l_configuration.name = PAMI_CLIENT_NUM_TASKS;
    result = PAMI_Client_query(*client, &l_configuration,1);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to query configuration.name=(%d): result = %d\n",
                l_configuration.name, result);
        return 1;
    }
    *num_tasks = l_configuration.value.intval;
    /* Docs04:  Create the client */

    /* Docs05:  Create the client */
    result = PAMI_Context_createv(*client, configuration, num_config, context, *num_contexts);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to create pami context: result = %d\n",
                result);
        return 1;
    }

    /* Docs06:  Create the client */
    return 0;
}

int pami_shutdown(pami_client_t        * client,          /* in/out:  client      */
        pami_context_t       * context,         /* in/out:  context     */
        size_t               * num_contexts)    /* in/out:  num_contexts*/
{
    pami_result_t result;
    /* Docs07:  Destroy the client and contexts */
    result = PAMI_Context_destroyv(context, *num_contexts);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to destroy pami context. result = %d\n", result);
        return 1;
    }

    result = PAMI_Client_destroy(client);
    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable to finalize pami client. result = %d\n", result);
        return 1;
    }
    /* Docs08:  Destroy the client and contexts*/
    return 0;
}

void decrement (pami_context_t   context,
        void           * cookie,
        pami_result_t    result)
{
    unsigned * value = (unsigned *) cookie;
    /*fprintf (stderr, "decrement() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
    --*value;
}

void done_fn (pami_context_t   context,
        void           * cookie,
        pami_result_t    result)
{
     cookie_t *value = (cookie_t *) cookie;
    /*fprintf (stderr, "decrement() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
    --(value->local);
}

void rdone_fn (pami_context_t   context,
        void           * cookie,
        pami_result_t    result)
{
     cookie_t *value = (cookie_t *) cookie;
    /*fprintf (stderr, "decrement() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
    --(value->remote);
}

void notify_fn(pami_context_t context, void *cookie, pami_result_t result)
{
    cookie_t *value = (cookie_t *)cookie;

    pami_send_immediate_t parameters;
    parameters.dispatch = BUFFER_READY_NOTIFICATION_DISPATCH_ID;
    parameters.header.iov_base = &value->forward;
    parameters.header.iov_len = sizeof(forward_t);
    parameters.data.iov_base  = NULL;
    parameters.data.iov_len = 0;
    parameters.dest = value->forward.proxyId;

    PAMI_Send_immediate(context, &parameters);
    /*printf("notify %d to send to %d\n", value->forward.proxyId, value->forward.destId);*/

    --(value->remote);
}

void recv_notification_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            * cookie,       /**< IN: dispatch cookie */
        const void      * header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      * data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     * recv)         /**< OUT: receive message structure */
{
    pami_task_t task;
    size_t offset;
    PAMI_Endpoint_query (origin, &task, &offset);

    //printf("got notification\n");
    forward_t *forward = (forward_t*)header;
    forwards_info_t *forwards_info = (forwards_info_t *)cookie;
    /*fprintf (stderr, "test_complete_fn() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
    //printf("cpindex = %d recv from %d on route %d forward %zu bytes at offset %zu to %zu at offset %zu\n", forwards_info->cpyindex, task, forward->routeId, forward->nbytes, forward->proxy_offset, forward->destId, forward->dest_offset);

    //forwards_info_t *forwards_info = (forwards_info_t *)cookie;
    memcpy(&(forwards_info->forwards[forwards_info->cpyindex]), header, sizeof(forward_t));

    //printf("cpyindex = %d recv from %d on route %d forward %zu bytes at offset %zu to %zu at offset %zu\n", forwards_info->cpyindex, task, forward->routeId, forward->nbytes, forward->proxy_offset, forward->destId, forward->dest_offset);

    forwards_info->cpyindex++;
    if(forwards_info->cpyindex == MAX_FORWARDS)
	forwards_info->cpyindex = 0;
    forwards_info->nforwards++;

    //printf("nforwards=%d, proindex=%d, cpyindex = %d\n", forwards_info->nforwards, forwards_info->proindex, forwards_info->cpyindex);
}

/**
 *  *  * \brief test completion dispatch function
 *   *   */
void test_complete_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            * cookie,       /**< IN: dispatch cookie */
        const void      * header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      * data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     * recv)         /**< OUT: receive message structure */
{
    unsigned * value = (unsigned *) cookie;
    /*fprintf (stderr, "test_complete_fn() cookie = %p, %d => %d\n", cookie, *value, *value - 1);*/
    --*value;
    return;
}
/**
 *  *  * \brief memory region exchange dispatch function
 *   *   */
void mr_exchange_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            * cookie,       /**< IN: dispatch cookie */
        const void      * header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      * data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     * recv)         /**< OUT: receive message structure */
{
    pami_task_t id;
    size_t offset;
    PAMI_Endpoint_query (origin, &id, &offset);

    mr_exchange_t * exchange = (mr_exchange_t *) cookie;
    memcpy(&exchange->task[id].mr, data, sizeof(pami_memregion_t));
    exchange->task[id].bytes = *((size_t *) header);
    exchange->counter--;
    return;
}
int exchange_mr(void *buf, size_t buf_size, pami_context_t context, int num_tasks, mr_exchange_t * exchange)
{
    pami_dispatch_callback_function fn;
    pami_dispatch_hint_t options = {};

    /* Initialize the exchange information */
    exchange->counter = num_tasks;

    fn.p2p = mr_exchange_fn;
    pami_result_t result = PAMI_Dispatch_set (context,
            MR_EXCHANGE_DISPATCH_ID,
            fn,
            (void *) exchange,
            options);

    if (result != PAMI_SUCCESS)
    {
        fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
        return 1;
    }

    /* Create a memory region for the local data buffer. */
    size_t bytes;
    pami_memregion_t mr;
    result = PAMI_Memregion_create(context, buf, buf_size, &bytes, &mr);

    if (result != PAMI_SUCCESS)
    {
        fprintf(stderr, "Error. Unable to create memory region. result = %d\n", result);
        return 1;
    }
    else if (bytes < buf_size)
    {
        fprintf(stderr, "Error. Unable to create memory region of a large enough size. result = %d\n", result);
        return 1;
    }

    /* Broadcast the data location to all tasks */
    for (size_t i = 0; i < num_tasks; i++)
    {
        pami_send_immediate_t parameters;
        parameters.dispatch        = MR_EXCHANGE_DISPATCH_ID;
        parameters.header.iov_base = (void *) & bytes;
        parameters.header.iov_len  = sizeof(size_t);
        parameters.data.iov_base   = (void *) & mr;
        parameters.data.iov_len    = sizeof(pami_memregion_t);
        PAMI_Endpoint_create (client, i, 0, &parameters.dest);

        result = PAMI_Send_immediate (context, &parameters);
    }

    /* Wait until all tasks have exchanged the memory region information */
    while (exchange->counter > 0)
        PAMI_Context_advance (context, 100);

    return 0;
}
