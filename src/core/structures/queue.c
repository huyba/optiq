#include <stdlib.h>
#include <string.h>
#include "../system/memory.h"
#include "queue.h"

void optiq_queue_pool_allocate(struct optiq_queue *pool, int num_elements, int content_size)
{
    pool->head = NULL;
    pool->tail = NULL;
    pool->size = num_elements;
    pool->content_size = content_size;

    int one_element_size = sizeof(struct optiq_element) + content_size;

    struct optiq_element *element;
    char *buffer = NULL;
    for (int i = 0; i < num_elements; i++) {
	buffer = (char *)core_memory_alloc(one_element_size, "buffer", "optiq_queue_pool_allocate");

	element = (struct optiq_element *)buffer;
	element->content = (void *)&buffer[sizeof(struct optiq_element)];

	if (pool->head == NULL) {
	    pool->head = element;
	    pool->head->next = NULL;
	} else {
	    element->next = pool->tail;
	    pool->tail->prev = element;
	}
	pool->tail = element;
	pool->tail->prev = NULL;
    }
}

void optiq_queue_init(struct optiq_queue *queue, int content_size)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->size = 0;
    queue->content_size = content_size;

    /*Create the pool for a number of availale element, avoiding memory allocation all the time*/
    optiq_queue_pool_allocate(queue->pool, NUM_OPTIQ_QUEUE_POOL_ELEMENTS, content_size);
}

void optiq_queue_enqueue(struct optiq_queue *queue, void *content)
{
    struct optiq_queue *pool = queue->pool;

    /*If there is NO element in the pool then allocate more*/
    if (pool->size == 0) {
	optiq_queue_pool_allocate(pool, NUM_OPTIQ_QUEUE_POOL_ELEMENTS, queue->content_size);
    }

    /* Get an element from the pool of queue */
    struct optiq_element *element = pool->head;
    if (pool->size == 1) {
	pool->head = NULL;
	pool->tail = NULL;
    } else {
	pool->head = pool->head->prev;
	pool->head->next = NULL;
    }
    pool->size--;

    /* Copy the element content into the element */
    memcpy(element->content, content, queue->content_size);

    /*Put the element into the queue*/
    if (queue->head == NULL) {
	queue->head = element;
	element->next = NULL;
    } else {
	element->next = queue->tail;
	queue->tail->prev = element;
    }

    queue->tail = element;
    element->prev = NULL;
    queue->size++;
}

int optiq_queue_dequeue(struct optiq_queue *queue, void *content)
{
    if (queue->size == 0) {
	content = NULL;
	return 0;
    }

    struct optiq_element *element = queue->head;

    /*Copy the content*/
    memcpy(content, element->content, queue->content_size);

    /*Update the queue*/
    if (queue->size == 1) {
	queue->head = NULL;
	queue->tail = NULL;
    } else {
	queue->head = queue->head->prev;
	queue->head->next = NULL;
    }
    queue->size--;

    /*Return the element back to pool*/
    element->next = queue->pool->tail;
    element->prev = NULL;
    queue->pool->tail->prev = element;
    queue->pool->tail = element;
    queue->pool->size++;
    
    return 1;
}
