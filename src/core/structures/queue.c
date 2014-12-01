#include <stdlib.h>
#include "queue.h"

void optiq_queue_init(struct optiq_queue *queue, int element_size)
{
    queue->head = NULL;
    queue->tail = NULL;
    queue->queue_size = 0;
    queue->element_size = element_size;
}

void optiq_queue_enqueue(struct optiq_queue *queue, void *element)
{

}
