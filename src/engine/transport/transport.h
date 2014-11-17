#ifndef OPTIQ_TRANSPORT_H
#define OPTIQ_TRANSPORT_H

#include <vector>

#include "transport_interface.h"

#include "../../core/structures/job.h"

#include "pami/pami_transport.h"
#include "gni/gni_transport.h"
#include "nonblk_mpi/nonblk_mpi_transport.h"

#define RECV_MESSAGE_SIZE (2*1024*1024)
#define NUM_RECV_MESSAGES 64
#define NUM_SEND_MESSAGES 64

using namespace std;

enum optiq_transport_type {
    PAMI = 1,
    GNI = 2,
    NONBLK_MPI = 3
};

struct optiq_transport {
    struct optiq_transport_interface *transport_implementation;
    void *concrete_transport;
    optiq_transport_type type;
    int size;
    int rank;
    vector<struct optiq_job> *jobs;

    vector<struct optiq_message *> in_use_recv_messages;
    vector<struct optiq_message *> avail_recv_messages;
    vector<struct optiq_message *> avail_send_messages;
};

void optiq_transport_init(struct optiq_transport *self, enum optiq_transport_type type);

int optiq_transport_send(struct optiq_transport *self, struct optiq_message *message);

int optiq_transport_recv(struct optiq_transport *self, struct optiq_message *message);

bool optiq_transport_test(struct optiq_transport *self, struct optiq_job *job);

int optiq_transport_destroy(struct optiq_transport *self);

void* optiq_transport_get_concrete_transport(struct optiq_transport *self);

#endif
