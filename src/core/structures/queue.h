#ifndef OPTIQ_QUEUE
#define OPTIQ_QUEUE

struct optiq_queue {
    void *head;
    void *tail;
    int queue_size;
    int element_size;
};

void optiq_queue_init(int element_size);
void optiq_queue_enqueue(struct optiq_queue *queue, void *element);

#endif
