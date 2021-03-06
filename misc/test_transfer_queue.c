#include<stdio.h>
#include<stdlib.h>

#define OPTIQ_WEIGHT_UNIT 16

struct optiq_message {
    char *buffer;
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
    virtual_lane->messages = message;
}

void optiq_send(void *buffer, int nbytes, int dest, MPI_Request *request)
{
    MPI_Isend(buffer, nbytes, MPI_BYTE, dest, 0, MPI_COMM_WORLD, request);
}

bool optiq_send_message_from_virtual_lane(struct optiq_virtual_lane* virtual_lane, struct optiq_vl_arbitration *vla)
{
    int index = 0;
    for (int i = 0; i < 4; i++) {
        if (vla[i].virtual_lane_id == virtual_lane->id) {
            index = i;
            break;
        }
    }
    struct optiq_message *message = virtual_lane->messages;
    optiq_isend(&message->buffer[message->current_offset], vla[i] * weight * OPTIQ_WEIGHT_UNIT, message->dest); 
}

int main(int argc, char **argv)
{
    printf("");

    /*Create a number of virual lanes*/
    int num_virtual_lanes = 4;
    struct optiq_virtual_lane *virtual_lanes = (struct optiq_virtual_lane *)malloc(sizeof(struct optiq_virtual_lane) * num_virtual_lanes);
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
        buffers[i] = (char *)malloc(length);
        service_level = i;
        struct optiq_message* message = optiq_create_message(buffers[i], length, dest, service_level, class_id);
        int vl = sl2vl[message->service_level];
        optiq_assign_message_to_vl(message, &virtual_lanes[i]);
    }

    bool done = false;
    while(!done) {
        /*Go through all the virtual lanes*/
        for (int i = 0; i < num_virtual_lanes; i++) {
            done = optiq_send_message_from_virtual_lane(&virtual_lanes[i], vla);
        }
    }

    return 0;
}
