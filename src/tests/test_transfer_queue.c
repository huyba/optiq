#include<stdio.h>
#include<stdlib.h>

#define OPTIQ_WEIGHT_UNIT 16

struct optiq_message {
    void *buffer;
    int length;
    int current_offset;
    int dest;
    int service_level;
    int class_id;
};

struct optiq_virtual_lane {
    int id;
    struct optiq_message *messages;
};

struct optiq_vl_arbitration {
    int virtual_lane_id;
    int weight;
    int priority;
    int current_weight;
    int max_weight;
};

struct service_level_to_vitual_lane {
    int service_level;
    int virtual_lane;
};

struct optiq_message* optiq_create_message(void *buffer, int length, int dest, int service_level, int class_id) 
{
    struct optiq_message* message = (struct optiq_message*)malloc(sizeof(struct optiq_message));
    message->buffer = buffer;
    message->length = length;
    message->current_offset = 0;
    message->dest = dest;
    message->service_level = service_level;
    message->class_id = class_id;
}

void optiq_assign_message_to_vl(struct optiq_message* message, struct optiq_virtual_lane* virtual_lane)
{
    int vl = sl2vl[message->service_level];
}

int main(int argc, char **argv)
{
    printf("");

    /*Create a number of virual lanes*/
    int num_virtual_lanes = 4;
    struct optiq_virtual_lanes *virtual_lanes = (struct optiq_virtual_lanes*)malloc(sizeof(struct optiq_virtual_lanes) * num_virtual_lanes);
    for (int i = 0; i < num_virtual_lanes; i++) {
        virtual_lanes[i].id = i;
    }

    /*Table of mapping between service level to virtual lanes*/
    int sl2vl[4];
    for (int i = 0; i < 4; i++) {
        sl2vl[i] = i;
    }

    /*Arbitration table to map virtual lane and weight and priority*/
    struct optiq_vl_arbitration *vla = (struct optiq_vl_arbitration *)malloc(sizeof(struct optiq_vl_arbitration) * 4);
    for (int i = 0; i < 4; i++) {
        vla[i].virtual_lane_id = i;
        vla[i].priority = 0;
        vla[i].weight = (i^2) * 128 * 1024 / OPTIQ_WEIGHT_UNIT;
        vla[i].current_weight = 0;
        vla[i].max_weight = 0;
    }

    /*Create a number of messages*/
    int num_messages = 4;
    char *buffers[4];
    int dest = 1, length = 0, service_level = 0, class_id = 0;
    for (int i = 0; i < num_messages; i++) {
        length = (i^2) * 1024 * 1024;
        buffers[i] = malloc(length);
        service_level = i;
        struct optiq_message* message = optiq_create_message(buffers[i], length, dest, service_level, class_id);
        int vl = sl2vl[message->service_level];
        optiq_assign_message_to_vl(message, virtual_lanes[i]);
    }

    return 0;

    
}
