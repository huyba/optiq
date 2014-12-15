#ifndef OPTIQ_QUEUE
#define OPTIQ_QUEUE

#define NUM_OPTIQ_QUEUE_POOL_ELEMENTS 128

struct optiq_element {
    struct optiq_element *next;
    struct optiq_element *prev;
    void *content;
};

struct optiq_queue {
    struct optiq_element *head;
    struct optiq_element *tail;
    int size;
    int content_size;
    struct optiq_queue *pool;
};

void optiq_queue_init(struct optiq_queue *queue, int content_size);

void optiq_queue_enqueue(struct optiq_queue *queue, void *content);

int optiq_queue_dequeue(struct optiq_queue *queue, void *content);

void optiq_queue_pool_allocate(struct optiq_queue *pool, int num_elements, int content_size);

#endif
