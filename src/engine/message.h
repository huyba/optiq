#ifndef OPTIQ_MESSAGE_H
#define OPTIQ_MESSAGE_H

#include <vector>

using namespace std;

struct optiq_message_header {
    int final_dest;
    int flow_id;
    int original_offset;
    int original_length;
};

struct optiq_message {
    struct optiq_message_header header;
    char *buffer;
    int length;
    int next_dest;
    int current_offset;
    int service_level;
};

struct optiq_message* get_send_message(vector<struct optiq_message *> *messages);

#endif
