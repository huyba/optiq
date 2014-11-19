#ifndef OPTIQ_MESSAGE_H
#define OPTIQ_MESSAGE_H

#include <vector>

#include "memory.h"

using namespace std;

struct optiq_message_header {
    int final_dest;
    int job_id;
    int flow_id;
    int original_source;
    int original_offset;
    int original_length;
};

struct optiq_message {
    struct optiq_message_header header;
    char *buffer;
    int length;
    int next_dest;
    int source;
    int current_offset;
    int service_level;
    int recv_length;
};

struct optiq_message* get_send_message(vector<struct optiq_message *> *messages);
struct optiq_message* get_message();
struct optiq_message* get_message_with_buffer(int buffer_size);
struct optiq_message** get_messages(int num_message);
struct optiq_message** get_messages_with_buffer(int num_messages, int buffer_size);
#endif
