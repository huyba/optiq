#ifndef OPTIQ_TRANSPORT_INTERFACE_H
#define OPTIQ_TRANSPORT_INTERFACE_H

enum tranport_type {
    PAMI = 1,
    UGNI = 2,
    NONBLK_MPI = 3
};

struct optiq_message {
    void *buffer;
    int size;
    int service_level;
};

struct transport_interface {
    void (*optiq_transport_init)();
    void (*optiq_transport_assign_service_level_to_message)(struct optiq_message  message, int service_level);
    void (*optiq_transport_add_message_to_hi_queue)(struct optiq_message message, int weight);
    void (*optiq_transport_add_message_to_low_queue)(struct optiq_message message, int weight);
};

#endif
