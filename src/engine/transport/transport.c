#include <stdlib.h>
#include <stdio.h>

#include "memory.h"
#include "transport.h"

void optiq_transport_init(struct optiq_transport *self, enum optiq_transport_type type)
{
    if (type == PAMI) {
	self->transport_implementation = &optiq_pami_transport_implementation;
    } else if (type == NONBLK_MPI) {
	self->transport_implementation = &optiq_nonblk_mpi_transport_implementation;
    } else if (type == GNI) {
	self->transport_implementation = &optiq_gni_transport_implementation;
    } else {
	/*self->transport_implementation = &optiq_tcp_ip_transport_implementation;*/
    }

    /*Init a number of messages with buffer for receiving incomming messages*/
    struct optiq_message **recv_messages = get_messages_with_buffer(NUM_RECV_MESSAGES, RECV_MESSAGE_SIZE);
    for (int i = 0; i < NUM_RECV_MESSAGES; i++) {
        self->avail_recv_messages.push_back(recv_messages[i]);
    }

    /*Init a number of messages without buffer for sending messages*/
    struct optiq_message **send_messages = get_messages(NUM_SEND_MESSAGES);
    for (int i = 0; i < NUM_SEND_MESSAGES; i++) {
        self->avail_send_messages.push_back(send_messages[i]);
    }

    self->type = type;
    self->concrete_transport = NULL;
    self->transport_implementation->init(self);
}

struct optiq_message* optiq_transport_get_send_message(struct optiq_transport *self)
{
    return get_send_message(&self->avail_send_messages);
}

void optiq_transport_return_send_message(struct optiq_transport *self, struct optiq_message *message)
{
    self->avail_send_messages.push_back(message);
}

int  optiq_transport_send(struct optiq_transport *self, struct optiq_message *message)
{
    return self->transport_implementation->send(self, message);
}

int optiq_transport_recv(struct optiq_transport *self, struct optiq_message *message)
{
    return self->transport_implementation->recv(self, message);
}

bool optiq_transport_test(struct optiq_transport *self, struct optiq_job *job)
{
    return self->transport_implementation->test(self, job);
}

int optiq_transport_destroy(struct optiq_transport *self)
{
    self->transport_implementation->destroy(self);

    free(self->concrete_transport);

    struct optiq_message *message;

    /*Free send_messages*/
    while (self->avail_send_messages.size() > 0) {
        message = self->avail_send_messages.back();
        self->avail_send_messages.pop_back();
        free(message);
    }

    /*Free recv_messages*/
    while (self->avail_recv_messages.size() > 0) {
        message = self->avail_recv_messages.back();
        self->avail_recv_messages.pop_back();
        free(message->buffer);
        free(message);
    }

    return 0;
}

void* optiq_transport_get_concrete_transport(struct optiq_transport *self)
{
    if (self->concrete_transport == NULL) {
	if (self->type == PAMI) {
	    struct optiq_pami_transport * pami_transport = (struct optiq_pami_transport *)core_memory_alloc(sizeof(struct optiq_pami_transport), "pami concrete transport", "get_concrete_transport");
	    pami_transport->transport = self;
	    self->concrete_transport = pami_transport;
	} else if (self->type == GNI) {
	    self->concrete_transport = (struct optiq_gni_transport *)core_memory_alloc(sizeof(struct optiq_gni_transport), "gni concrete transport", "get_concrete_transport");
	} else if (self->type == NONBLK_MPI) {
	    self->concrete_transport = (struct optiq_nonblk_mpi_transport *)core_memory_alloc(sizeof(struct optiq_nonblk_mpi_transport), "nonblk concrete transport", "get_concrete_transport");
	} else {

	}
    }

    return self->concrete_transport;
}

void optiq_transport_assign_jobs(struct optiq_transport *self, vector<struct optiq_job> *jobs)
{
    self->jobs = jobs;
    return self->transport_implementation->assign_jobs(self, jobs);
}


void optiq_transport_assign_virtual_lanes(struct optiq_transport *self, vector<struct optiq_virtual_lane> *virtual_lanes, vector<optiq_arbitration> *arbitration_table)
{
    self->virtual_lanes = virtual_lanes;
    self->arbitration_table = arbitration_table;
    self->transport_implementation->assign_virtual_lanes(self, virtual_lanes, arbitration_table);
}
