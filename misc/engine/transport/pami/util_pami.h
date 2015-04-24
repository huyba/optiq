#ifndef UTIL_H
#define UTIL_H

#include <pami.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>

#include <hwi/include/bqc/A2_inlines.h>

#define TEST_COMPLETE_DISPATCH_ID 10
#define MR_EXCHANGE_DISPATCH_ID   11
#define BUFFER_READY_NOTIFICATION_DISPATCH_ID 12
#define MESSAGE_READY_NOTIFICATION_DISPATCH_ID 14
#define MESSAGE_TRANSFER_COMPLETE_DISPATCH_ID 15
#define MESSAGE_RECV_ACK_DISPATCH_ID 16

#define MAX_FORWARDS 128*1024

#define PAMI_ENDPOINT_INFO(endpoint,task,offset) { task = endpoint & 0x007fffff; offset = (endpoint >> 23) & 0x03f; }

typedef struct
{
  pami_memregion_t mr;
  size_t           bytes;
} mr_t;

typedef struct
{
  volatile size_t counter;
  mr_t            task[0];
} mr_exchange_t;

typedef struct
{
  pami_task_t task;
  char        name[1024];
} testinfo_t;

typedef struct
{
    size_t destId;
    size_t dest_offset;
    size_t nbytes;
    size_t proxyId;
    size_t proxy_offset;
    size_t routeId;
} forward_t;

typedef struct
{
    volatile size_t nforwards;
    volatile size_t cpyindex;
    volatile size_t proindex;
    forward_t forwards[0];
} forwards_info_t;

typedef struct
{
    volatile size_t remote;
    volatile size_t local;
    forward_t forward;
} cookie_t;

int pami_init(pami_client_t        * client,          /* in/out:  client      */
        pami_context_t       * context,         /* in/out:  context     */
        char                 * clientname,      /* in/out:  clientname  */
        size_t               * num_contexts,    /* in/out:  num_contexts*/
        pami_configuration_t * configuration,   /* in:      config      */
        size_t                 num_config,      /* in:      num configs */
        pami_task_t          * task_id,         /* out:     task id     */
        size_t               * num_tasks);       /* out:     num tasks   */

int pami_shutdown(pami_client_t        * client,          /* in/out:  client      */
        pami_context_t       * context,         /* in/out:  context     */
        size_t               * num_contexts);    /* in/out:  num_contexts*/

void decrement (pami_context_t   context,
        void           * cookie,
        pami_result_t    result);

void done_fn (pami_context_t   context,
        void           * cookie,
        pami_result_t    result);

void rdone_fn (pami_context_t   context,
        void           * cookie,
        pami_result_t    result);

void notify_fn(pami_context_t context, void *cookie, pami_result_t result);

void recv_notification_fn (
        pami_context_t    context,      /**< IN: PAMI context */
        void            * cookie,       /**< IN: dispatch cookie */
        const void      * header,       /**< IN: header address */
        size_t            header_size,  /**< IN: header size */
        const void      * data,         /**< IN: address of PAMI pipe buffer */
        size_t            data_size,    /**< IN: size of PAMI pipe buffer */
        pami_endpoint_t   origin,
        pami_recv_t     * recv);        /**< OUT: receive message structure */

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
        pami_recv_t     * recv);        /**< OUT: receive message structure */

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
        pami_recv_t     * recv);         /**< OUT: receive message structure */

int exchange_mr(void *buf, size_t buf_size, pami_context_t context, int num_tasks, mr_exchange_t * exchange);

#endif
