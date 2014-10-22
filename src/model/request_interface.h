#ifndef OPTIQ_REQUEST_INTERFACE
#define OPTIQ_REQUEST_INTERFACE

enum optiq_request_type {
    COMM = 0,
    IO = 1
};

struct optiq_request {
    int source;
    int destination;
    int data_size;
    int batch_id;
    optiq_request_type type;
};

struct optiq_request_interface {
    void (*optiq_request_comm_add)(int source, int dest, int size, void *buf);
    void (*optiq_request_io_add)(int source, int size, void *buf);
    void (*optiq_request_collect)(int num_request, optiq_request *requests);
};



#endif
