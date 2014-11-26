#include <stdio.h>

#include "message.h"
#include "virtual_lane.h"
#include "transport.h"

void print_arbitration_table(vector<struct optiq_arbitration> &ab)
{
    int num_entries = ab.size();
    printf("Arbitration table: #entries = %d\n", num_entries);
    printf("vl_id weight priority\n");
    for (int i = 0; i < num_entries; i++) {
	printf("%d %d %d\n", ab[i].virtual_lane_id, ab[i].weight, ab[i].priority);
    }
}

void print_virtual_lanes(map<int, struct optiq_virtual_lane> &virtual_lanes)
{
    int num_virtual_lanes = virtual_lanes.size();

    printf("Current status of virtual lanes: \n");
    printf("Number of virtual lanes = %d\n", num_virtual_lanes);
    for (map<int, struct optiq_virtual_lane>::iterator iter = virtual_lanes.begin(); iter != virtual_lanes.end(); ) {
	printf("Virtual lane id = %d, #messages = %lu\n", iter->first, iter->second.requests.size());

	for (int j = 0; j < iter->second.requests.size(); j++) {
	    printf("Message has %d bytes\n", iter->second.requests[j]->length);
	}
    }
}

void optiq_vlab_transport(struct optiq_vlab &vlab, struct optiq_transport *transport)
{
    /*Iterate the arbitration table to get the next virtual lane*/
    bool empty = false;
    int nbytes = 0;
    int virtual_lane_id = 0;
    struct optiq_virtual_lane *virtual_lane = NULL;
    map<int, struct optiq_virtual_lane>::iterator iter;

    while (!empty) {

	empty = true;

	/*Get the virtual lane id*/
	for (int index = 0; index < vlab.ab.size(); index++) {
	    virtual_lane_id = vlab.ab[index].virtual_lane_id;

	    iter = vlab.vl.find(virtual_lane_id);

	    /*Get the virtual lane with that id*/
	    if (iter != vlab.vl.end()) {
		virtual_lane = &iter->second;
		if (virtual_lane->requests.size() > 0) {
		    struct optiq_message *message = virtual_lane->requests.front();

#ifdef DEBUG
		    printf("Rank %d virtual_lane_id = %d, quota= %d, message length = %d, offset = %d, original_offset = %d\n", transport->rank, virtual_lane_id, vlab.ab[index].weight * BASE_UNIT_SIZE, message->length, message->current_offset, message->header.original_offset); 
#endif
		    nbytes = vlab.ab[index].weight * BASE_UNIT_SIZE;

		    if (message->current_offset + nbytes >= message->length) {
			nbytes = message->length - message->current_offset;
		    }

		    struct optiq_message *instant = optiq_transport_get_send_message(transport);
		    instant->buffer = &message->buffer[message->current_offset];
		    instant->length = nbytes;
		    instant->current_offset = 0;
		    instant->next_dest = message->next_dest;
		    instant->source = transport->rank;
		    instant->header = message->header;
		    instant->header.original_offset += message->current_offset;

#ifdef DEBUG
		    printf("Rank %d send a message with offset %d\n", transport->rank, instant->header.original_offset);
#endif
		    optiq_transport_send(transport, instant);

		    empty = false;

		    /*Update the virtual lanes and message*/
		    if (message->current_offset + nbytes >= message->length) {
                        virtual_lane->requests.erase(virtual_lane->requests.begin());
                        optiq_transport_return_send_message(transport, message);
		    } else {
			virtual_lane->requests.front()->current_offset += nbytes;
		    }
		}
	    }
	}
    }
}

int optiq_vlab_create(struct optiq_vlab &vlab, vector<struct optiq_job> &jobs, int world_rank)
{
    const struct optiq_flow *flow = NULL;

    for (int i = 0; i < jobs.size(); i++) {
	for (int j = 0; j < jobs[i].flows.size(); j++) {
	    flow = &jobs[i].flows[j];
	    for (int k = flow->arcs.size()-1; k >= 0; k--) {
		if (flow->arcs[k].ep1 == world_rank) {
		    struct optiq_arbitration ab;
		    struct optiq_virtual_lane vl;

		    ab.virtual_lane_id = flow->id;
		    ab.weight = flow->throughput;
		    ab.priority = 0;
		    vl.id = flow->id;

		    vlab.ab.push_back(ab);
		    vlab.vl.insert(make_pair(vl.id, vl));
		}
	    }
	}
    }
}

int optiq_vlab_add_message(struct optiq_vlab &vlab, struct optiq_message *message)
{
    map<int, struct optiq_virtual_lane>::iterator iter = vlab.vl.find(message->header.flow_id);

    if (iter != vlab.vl.end()) {
	iter->second.requests.push_back(message);
	return 0;
    } 

    return 1;
}

int optiq_vlab_add_job(struct optiq_vlab &vlab, struct optiq_job &job, struct optiq_transport *transport)
{
    char *buffer = (char *)job.buffer;
    int data_size = job.demand;

    int total_local_throughput = 0;

    for (int i = 0; i < job.flows.size(); i++) {
	/*Compute the total flows for the local node*/
	total_local_throughput += job.flows[i].throughput;
    }

    /*Fill in the virtual lanes with data from local jobs*/
    int global_offset = 0, length = 0;
    for (int i = 0; i < job.flows.size(); i++) {
	length = ((double)job.flows[i].throughput / (double)total_local_throughput) * (double)data_size;
	struct optiq_message *message = optiq_transport_get_send_message(transport);

	if (i == job.flows.size() - 1) {
	    length = data_size - global_offset;
	}
	message->header.original_length = data_size;
	message->header.original_offset = global_offset;
	message->header.job_id = job.id;
	message->header.flow_id = job.flows[i].id;
	message->header.final_dest = job.dest;
	message->header.original_source = job.source;
	message->next_dest = transport->next_dest[job.flows[i].id];
	message->source = job.source;
	message->current_offset = 0;
	message->service_level = 0;
	message->buffer = &buffer[global_offset];
	message->length = length;
	global_offset += length;

#ifdef DEBUG
	printf("Rank %d add message with orignal offset = %d to VL\n", transport->rank, message->header.original_offset);
#endif
	optiq_vlab_add_message(vlab, message);
	job.flows[i].message = message;
	job.flows[i].sent_bytes = 0;
    }

    return 0;
}
