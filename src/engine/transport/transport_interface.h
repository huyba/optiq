#ifndef OPTIQ_TRANSPORT_INTERFACE
#define OPTIQ_TRANSPORT_INTERFACE

struct optiq_message {
    void *buffer;
    int size;
    int service_level;
};

struct transport_interface {
    void (*optiq_transport_assign_service_level_to_message)(struct optiq_message  message, int service_level);
    void (*optiq_transport_add_message_to_hi_queue)(struct optiq_message message, int weight);
    void (*optiq_transport_add_message_to_low_queue)(struct optiq_message message, int weight);
};

#endif
