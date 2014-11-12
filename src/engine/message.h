#ifndef OPTIQ_MESSAGE_H
#define OPTIQ_MESSAGE_H

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

#endif
