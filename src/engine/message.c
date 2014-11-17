#include <vector>
#include <stdlib.h>

#include "message.h"

struct optiq_message* get_send_message(vector<struct optiq_message *> *messages)
{
    struct optiq_message *message = NULL;

    if ((*messages).size() > 0) {
        message = (*messages).back();
        (*messages).pop_back();
    } else {
        message = (struct optiq_message *)malloc(sizeof(struct optiq_message));
    }

    return message;
}

struct optiq_message *get_message()
{
    struct optiq_message *message = (struct optiq_message *)core_memory_alloc(sizeof(struct optiq_message), "message", "get_message");

    message->length = 0;
    message->buffer = NULL;
    message->current_offset = 0;
    message->service_level = 0;
    message->next_dest = 0;
    message->recv_length = 0;
    message->header.final_dest = -1;
    message->header.flow_id = -1;
    message->header.original_length = 0;
    message->header.original_offset = 0;

    return message;
}

struct optiq_message** get_messages(int num_messages)
{
    struct optiq_message **messages = (struct optiq_message **)core_memory_alloc(sizeof(struct optiq_message *) * num_messages, "messages", "get_messages");

    for (int i = 0; i < num_messages; i++) {
        messages[i] = get_message();
    }

    return messages;
}

struct optiq_message* get_message_with_buffer(int buffer_size)
{
    struct optiq_message *message = (struct optiq_message *)core_memory_alloc(sizeof(struct optiq_message), "message", "get_message");

    message->length = buffer_size;
    message->buffer = (char *)core_memory_alloc(buffer_size, "message->buffer", "get_message_with_buffer");
    message->current_offset = 0;
    message->service_level = 0;
    message->next_dest = 0;
    message->recv_length = 0;
    message->header.final_dest = -1;
    message->header.flow_id = -1;
    message->header.original_length = 0;
    message->header.original_offset = 0;

    return message;
}

struct optiq_message** get_messages_with_buffer(int num_messages, int buffer_size)
{
    struct optiq_message **messages = (struct optiq_message **)core_memory_alloc(sizeof(struct optiq_message *) * num_messages, "messages", "get_messages_with_buffer");

    for (int i = 0; i < num_messages; i++) {
        messages[i] = get_message_with_buffer(buffer_size);
    }

    return messages;
}
