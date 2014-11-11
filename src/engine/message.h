#ifndef OPTIQ_MESSAGE_H
#define OPTIQ_MESSAGE_H

struct optiq_message_header {
    int final_dest;
    int flow_id;
    int original_offset;
    int original_length;
};

struct optiq_message {
    char *buffer;
    int next_dest;
    int service_level;
    int length;
    int current_offset;
    struct optiq_message_header header;
};

#endif
