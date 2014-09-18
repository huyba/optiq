#include <math.h>
#include "util_pami.h"
#include "mfbgq.h"

#include <mpi.h>

/* Needed for the various Personality functions*/
#include <spi/include/kernel/location.h>
#include <spi/include/kernel/process.h>
#include <firmware/include/personality.h>

#define ITERATIONS 100
#define BUFFERSIZE 128*1024*1024
#define MIN_SIZE 1024

int calculate_winsize(int message_size)
{
    if (message_size < 1024)
	return message_size;
    else if(1024 <= message_size && message_size < 2048)
	return 1024;
    if (2048 <= message_size && message_size < 4096)
	return 2048;
    if (4096 <= message_size && message_size < 8192)
	return 4096;
    if (8192 <= message_size && message_size < 16384)
	return 4096;
    if (16384 <= message_size && message_size < 32768)
	return 4096;
    if (32768 <= message_size && message_size < 65536)
	return 8192;
    if (65536 <= message_size && message_size < 131072)
	return 16384;
    if (131072 <= message_size && message_size < 262144)
	return 16384;
    if (262144 <= message_size && message_size < 524288)
	return 16384;
    if (524288 <= message_size && message_size < 1048576)
	return 32768;
    if (1048576 <= message_size && message_size < 2097152)
	return 65536;
    if (2097152 <= message_size && message_size < 4194304)
	return 65536;
    if (4194304 <= message_size && message_size < 8388608)
	return 65536;
    if (8388608 <= message_size && message_size < 16777216)
	return 131072;
    if (16777216 <= message_size && message_size < 33554432)
	return 131072;
    if (33554432 <= message_size && message_size < 67108864)
	return 262144;
    if (67108864 <= message_size && message_size < 134217728)
	return 262144;
    if (134217728 <= message_size)
	return 524288;
}

int main (int argc, char ** argv)
{
    int world_rank, world_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    Personality_t pers;
    Kernel_GetPersonality(&pers, sizeof(pers));

    int dims[5], coords[5];
    bool isTorus[5];

    dims[0] = pers.Network_Config.Anodes; coords[0] = pers.Network_Config.Acoord;
    dims[1] = pers.Network_Config.Bnodes; coords[1] = pers.Network_Config.Bcoord;
    dims[2] = pers.Network_Config.Cnodes; coords[2] = pers.Network_Config.Ccoord;
    dims[3] = pers.Network_Config.Dnodes; coords[3] = pers.Network_Config.Dcoord;
    dims[4] = pers.Network_Config.Enodes; coords[4] = pers.Network_Config.Ecoord;

    int nsources = 1;
    int ndests = 1;
    int *sourceId = (int*)malloc(sizeof(int)*nsources);
    int *destId = (int *)malloc(sizeof(int)*ndests);

    int source[5], dest[5];

    for(int i =0; i < 5; i++)
    {
	source[i] = 0;
	dest[i] = 2;
    }
    dest[2] = 2;
    dest[3] = 2;
    dest[4] = 0;

    //size_t sourceId = compute_nid(num_dims, source, dims);
    //int destId = compute_nid(num_dims, dest, dims);

    sourceId[0] = 10;
    destId[0] = 20;

    if(argc == 4)
    {
	sourceId[0] = atoi(argv[2]);
        destId[0] = atoi(argv[3]);
    }

    int myDestId = destId[0];
    int mySourceId = sourceId[0];

    int max_paths = nsources*10;
    multipath_t *mp = (multipath_t*)malloc(2*sizeof(size_t) + sizeof(path_t)*max_paths);

    mp->num_paths = 0;

    for(int i = 0; i < 10; i++)
	mp->paths[i].num_vertices = 0;

    int num_dims = 5;

    int max_flow = compute_max_flow(num_dims, dims, sourceId, nsources, destId, ndests, mp);

    if(world_rank == 0)
	print_path(mp);

    setbuf (stdout, NULL);
    setbuf (stderr, NULL);

    pami_client_t        client;
    pami_context_t       context;
    size_t               num_contexts = 1;
    pami_task_t          me;
    size_t               num_tasks;
    pami_result_t        result;

    int rc = pami_init (&client,        /* Client             */
	    &context,       /* Context            */
	    NULL,           /* Clientname=default */
	    &num_contexts,  /* num_contexts       */
	    NULL,           /* null configuration */
	    0,              /* no configuration   */
	    &me,            /* task id            */
	    &num_tasks);    /* number of tasks    */

    if (rc == 1)
	return 1;

    /* ------------------------------------------------------------------------ */
    /* Set up the 'test completion' dispatch function.                          */
    /* ------------------------------------------------------------------------ */
    volatile unsigned test_active = (me == 0) ? num_tasks - 1 : 1;
    pami_dispatch_callback_function fn;
    fn.p2p = test_complete_fn;
    pami_dispatch_hint_t options = {};

    result = PAMI_Dispatch_set (context,
	    TEST_COMPLETE_DISPATCH_ID,
	    fn,
	    (void *) & test_active,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch function \"test_complete_fn()\" with dispatch id %d. result = %d\n", TEST_COMPLETE_DISPATCH_ID, result);
	return 1;
    }

    volatile unsigned recv_ack = 1;
    fn.p2p = test_complete_fn;
    result = PAMI_Dispatch_set (context,
	    MESSAGE_RECV_ACK_DISPATCH_ID,
	    fn,
	    (void *) &recv_ack,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch function \"test_complete_fn()\" with dispatch id %d. result = %d\n", TEST_COMPLETE_DISPATCH_ID, result);
	return 1;
    }

    /* ------------------------------------------------------------------------ */
    /* Set up the 'recv notification' dispatch function.                          */
    /* ------------------------------------------------------------------------ */
    forwards_info_t *forwards_info = (forwards_info_t*)malloc(sizeof(forwards_info_t) + sizeof(forward_t)*MAX_FORWARDS);
    forwards_info->cpyindex = 0;
    forwards_info->proindex = 0;
    forwards_info->nforwards = 0;
    fn.p2p = recv_notification_fn;

    result = PAMI_Dispatch_set (context,
	    BUFFER_READY_NOTIFICATION_DISPATCH_ID,
	    fn,
	    (void *)forwards_info,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch function \"recv_notification_fn()\" with dispatch id %d. result = %d\n", BUFFER_READY_NOTIFICATION_DISPATCH_ID, result);
	return 1;
    }
    /* ------------------------------------------------------------------------ */

    volatile unsigned remaining_transfers = 0;
    fn.p2p = test_complete_fn;
    result = PAMI_Dispatch_set (context,
	    MESSAGE_READY_NOTIFICATION_DISPATCH_ID,
	    fn,
	    (void *)&remaining_transfers,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch function \"remaining transfer()\" with dispatch id %d. result = %d\n", BUFFER_READY_NOTIFICATION_DISPATCH_ID, result);
	return 1;
    }

    volatile unsigned transfer_complete = 1;
    fn.p2p = test_complete_fn;
    result = PAMI_Dispatch_set (context,
	    MESSAGE_TRANSFER_COMPLETE_DISPATCH_ID,
	    fn,
	    (void *)&transfer_complete,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch function \"transfer_complete_fn()\" with dispatch id %d. result = %d\n", MESSAGE_TRANSFER_COMPLETE_DISPATCH_ID, result);
	return 1;
    }

    mr_exchange_t * exchange =
	(mr_exchange_t *) malloc (sizeof(mr_exchange_t) + sizeof(mr_t) * num_tasks);

    /* Initialize the exchange information */
    exchange->counter = num_tasks;

    mr_t * info = exchange->task;

    fn.p2p = mr_exchange_fn;
    result = PAMI_Dispatch_set (context,
	    MR_EXCHANGE_DISPATCH_ID,
	    fn,
	    (void *) exchange,
	    options);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable register pami dispatch. result = %d\n", result);
	return 1;
    }


    /* Allocate and intialize the local data buffer for the test. */
    uint8_t * local = (uint8_t *) malloc (BUFFERSIZE);
    if(me == 0)
	memset(local, 7, BUFFERSIZE);
    else
	memset(local, 0, BUFFERSIZE);

    /* Create a memory region for the local data buffer. */
    size_t bytes;
    pami_memregion_t mr;
    result = PAMI_Memregion_create (context, (void *) local, BUFFERSIZE,
	    &bytes, &mr);

    if (result != PAMI_SUCCESS)
    {
	fprintf (stderr, "Error. Unable to create memory region. result = %d\n", result);
	return 1;
    }
    else if (bytes < BUFFERSIZE)
    {
	fprintf (stderr, "Error. Unable to create memory region of a large enough size. result = %d\n", result);
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

    /* **************************************************************************
     * Begin the test - Task 0 will use PAMI_Rput to performance test against
     * the target tasks specified on the command line.
     * **************************************************************************/

    route_t routes[9];
    int nroutes = 0;

    bool isSource = false;
    bool isProxy = false;
    bool isDest = false;

    /*Determine who is who and kind-of routing table*/
    for(int i = 0; i < mp->num_paths; i++)
    {
	path_t path = mp->paths[i];
	for(int j = 0; j < path.num_vertices; j++)
	{
	    /*Source nodes*/
	    if(me == path.vertices[j] && j == 0)
		isSource = true;
	    else if(me == path.vertices[j] && j == path.num_vertices-1)
		isDest = true;
	    else if(me == path.vertices[j])
		isProxy = true;

	    if(me == path.vertices[j])
	    {
		routes[nroutes].routeId = i;
		if(j+1 < path.num_vertices)
		    routes[nroutes].next = path.vertices[j+1];
		nroutes++;
	    }
	}
    }

    /*Create endpoint*/
    pami_rput_simple_t parameters[9];
    cookie_t cookies[9];

    volatile unsigned active;
    if(isSource || isProxy)
    {
	for(int i = 0; i < nroutes; i++)
	{
	    parameters[i].rma.hints          = (pami_send_hint_t) {0};
	    parameters[i].rma.cookie         = (void *)&cookies[i];//& active;
	    parameters[i].rma.done_fn        = done_fn;//decrement;
	    parameters[i].rdma.local.mr      = &info[me].mr;
	    parameters[i].rdma.local.offset  = 0;
	    parameters[i].rdma.remote.offset = 0;
	    parameters[i].put.rdone_fn = /*rdone_fn;*/notify_fn;//NULL;
	    parameters[i].rdma.remote.mr = &info[routes[i].next].mr;
	    PAMI_Endpoint_create (client, routes[i].next, 0, &parameters[i].rma.dest);

	    cookies[i].forward.proxyId = routes[i].next;
	    cookies[i].forward.destId = myDestId;
	    cookies[i].forward.dest_offset = 0;
	    cookies[i].forward.proxy_offset = 0;
	    cookies[i].forward.routeId = routes[i].routeId;
	    cookies[i].local = 0;
	    cookies[i].remote = 0;
	}
    }

    uint64_t t0, t1;

    size_t min_size = MIN_SIZE;
    if(argc > 1)
    {
	nroutes = (nroutes < atoi(argv[1]) ? nroutes : atoi(argv[1]));
	if(isSource)
	    printf("Using %d first paths.\n", nroutes);
    }

    /* Source node send data to proxies */
    if (isSource)
    {
	double sec, bw;
	for (size_t nbytes = min_size; nbytes <= BUFFERSIZE; nbytes*=2)
	{
	    int rsize = nbytes/nroutes;
	    int win_size = calculate_winsize(rsize);
	    int ntransfers = (rsize/win_size > 1 ? rsize/win_size : 1);
	    t0 = GetTimeBase();
	    for(int iter = 0; iter < ITERATIONS; iter++)
	    {
		/*Split and put a message*/
		recv_ack = 1;
		for(int t = 0; t < ntransfers; t++)
		{
		    for (int j = 0; j < nroutes; j++)
		    {
			size_t offset = nbytes/nroutes*j + t*win_size;
			cookies[j].local = 1;
			cookies[j].remote = 1;
			cookies[j].forward.nbytes = win_size;
			cookies[j].forward.proxyId = routes[j].next;
			cookies[j].forward.proxy_offset = offset;
			cookies[j].forward.destId = myDestId;
			cookies[j].forward.dest_offset = offset;
			cookies[j].forward.routeId = routes[j].routeId;

			parameters[j].rdma.remote.offset = offset;
			parameters[j].rdma.local.offset  = offset;
			parameters[j].rma.bytes      = win_size;

			PAMI_Rput (context, &parameters[j]);
			//printf("%d Put %d bytes at offset %d to %d at offset %d\n", me, parameters[j].rma.bytes, parameters[j].rdma.local.offset, parameters[j].rma.dest, parameters[j].rdma.remote.offset);
		    }

		    /*Wait for all message to be ready at proxies*/
		    for(int j = 0; j < nroutes; j++)
		    {
			while (cookies[j].remote > 0)
			    PAMI_Context_advance (context, 100);
		    }
		}
		/*Wait till message received at dest*/
		while (recv_ack > 0)
		    PAMI_Context_advance (context, 100);
	    }

	    t1 = GetTimeBase();
	    sec = (double)(t1-t0)/1.6e3/1e6/ITERATIONS;
	    bw = (double)nbytes/1024/1024/sec;
	    //printf("%zu %8.6f %8.4f\n", nbytes, sec, bw);
	}
    }

    /*Proxies forward data to destination*/
    if(isProxy)
    {
	while(true)
	{
	    while (forwards_info->nforwards == 0)
		PAMI_Context_advance (context, 100);
	    forwards_info->nforwards--;

	    /*Put data to destination*/
	    forward_t forward = forwards_info->forwards[forwards_info->proindex];

	    for (int j = 0; j < nroutes; j++)
	    {
		if(routes[j].routeId == forward.routeId)
		{
		    cookies[j].remote = 1;
		    cookies[j].forward.nbytes = forward.nbytes;
		    cookies[j].forward.proxyId = routes[j].next;
		    cookies[j].forward.proxy_offset = forward.proxy_offset;
		    cookies[j].forward.destId = forward.destId;
		    cookies[j].forward.dest_offset = forward.dest_offset;
		    cookies[j].forward.routeId = forward.routeId;

		    parameters[j].rdma.local.offset  = forward.proxy_offset;
		    parameters[j].rdma.remote.offset  = forward.proxy_offset;
		    parameters[j].rma.bytes      = forward.nbytes;

		    PAMI_Rput (context, &parameters[j]);
		    forwards_info->proindex++;
		    if(forwards_info->proindex == MAX_FORWARDS)
			forwards_info->proindex = 0;
		}
	    }
	}
    }

    /*Destination wait util get all result*/
    if(isDest)
    {
	pami_send_immediate_t parameters;
	parameters.dispatch = MESSAGE_RECV_ACK_DISPATCH_ID;
	parameters.header.iov_base = (void*)&recv_ack;
	parameters.header.iov_len = sizeof(size_t);
	parameters.data.iov_base  = NULL;
	parameters.data.iov_len = 0;
	parameters.dest = mySourceId;

	double bw[1024], sec[1024];
	int index = 0;
	for (size_t nbytes = min_size; nbytes <= BUFFERSIZE; nbytes*=2)
	{
	    int rsize = nbytes/nroutes;
	    int win_size = calculate_winsize(rsize);
	    int ntransfers = (rsize/win_size > 1 ? rsize/win_size : 1);

	    t0 = GetTimeBase();
	    for(int iter = 0; iter < ITERATIONS; iter++)
	    {
		for(int t = 0; t < ntransfers; t++)
		{
		    for (int j = 0; j < nroutes; j++)
		    {
			while (forwards_info->nforwards == 0)
			    PAMI_Context_advance (context, 100);
			forwards_info->nforwards--;
		    }
		}
		/*Ack that message received*/
		PAMI_Send_immediate(context, &parameters);
	    }

	    t1 = GetTimeBase();
	    sec[index] = (double)(t1-t0)/1.6e3/1e6/ITERATIONS;
	    bw[index] = (double)nbytes/1024/1024/sec[index];
	    index++;
	}

	size_t nbytes = min_size;
	printf("At dest\n");
	int j = 0;
	for (size_t nbytes = min_size; nbytes <= BUFFERSIZE; nbytes*=2)
	{
	    printf("%zu %8.6f %8.4f\n", nbytes, sec[j], bw[j]);
	    j++;
	}
    }

    /*Task 0 lets other tasks that test is completed*/
    if(me == 0)
    {
	pami_send_t parameters;
	parameters.send.dispatch        = TEST_COMPLETE_DISPATCH_ID;
	parameters.send.header.iov_base = (void *) & test_active;
	parameters.send.header.iov_len  = 1;
	parameters.send.data.iov_base   = (void *) & test_active;
	parameters.send.data.iov_len    = 1;
	parameters.events.cookie        = (void *) & test_active;
	parameters.events.local_fn      = decrement;
	parameters.events.remote_fn     = NULL;

	for (size_t i = 1; i < num_tasks; i++)
	{
	    PAMI_Endpoint_create (client, i, 0, &parameters.send.dest);
	    result = PAMI_Send (context, &parameters);
	}
    }
    /* Wait until the test is completed */
    while (test_active)
	PAMI_Context_advance (context, 100);

    if(isDest)
    {
	uint8_t *buf = (uint8_t*)malloc(BUFFERSIZE);
	memset(buf, 7, BUFFERSIZE);
	if(memcmp((void*)buf, (void*)local, BUFFERSIZE) != 0)
	    printf("%d Error on transfer\n", me);
	else
	    printf("%d Transfer well-done\n", me);
	//for(int i = 0; i < BUFFERSIZE; i++)
	  //  if(buf[i] != local[i])
	//	printf("At %d %d!=%d",i, local[i], buf[i]);
	//printf("\n");
    }

    rc = pami_shutdown(&client, &context, &num_contexts);

    if (rc == 1)
	return 1;

    return 0;
};
