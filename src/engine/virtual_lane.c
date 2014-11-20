#include <stdio.h>

#include "message.h"
#include "virtual_lane.h"
#include "transport.h"

void print_arbitration_table(vector<struct optiq_arbitration> ab)
{
    int num_entries = ab.size();
    printf("Arbitration table: #entries = %d\n", num_entries);
    printf("vl_id weight priority\n");
    for (int i = 0; i < num_entries; i++) {
        printf("%d %d %d\n", ab[i].virtual_lane_id, ab[i].weight, ab[i].priority);
    }
}

void print_virtual_lanes(vector<struct optiq_virtual_lane> &virtual_lanes)
{
    int num_virtual_lanes = virtual_lanes.size();

    printf("Current status of virtual lanes: \n");
    printf("Number of virtual lanes = %d\n", num_virtual_lanes);
    for (int i = 0; i < num_virtual_lanes; i++) {
        printf("Virtual lane id = %d, #messages = %lu\n", virtual_lanes[i].id, virtual_lanes[i].requests.size());

        for (int j = 0; j < virtual_lanes[i].requests.size(); j++) {
            printf("Message has %d bytes\n", virtual_lanes[i].requests[j]->length);
        }
    }
}

void transport_from_virtual_lanes(struct optiq_transport *transport, vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table)
{
    int num_entries_arb_table = arbitration_table.size();
    int num_virtual_lanes = virtual_lanes.size();

    /*Iterate the arbitration table to get the next virtual lane*/
    bool done = false;
    int nbytes = 0;
    int virtual_lane_id = 0;

    while (!done) {

        done = true;

        /*Get the virtual lane id*/
        for (int index = 0; index < num_entries_arb_table; index++) {
            virtual_lane_id = arbitration_table[index].virtual_lane_id;

            /*Get the virtual lane with that id*/
            for (int i = 0; i < num_virtual_lanes; i++) {
                if (virtual_lanes[i].id == virtual_lane_id) {
                    if (virtual_lanes[i].requests.size() > 0) {
                        struct optiq_message *message = virtual_lanes[i].requests.front();

                        //printf("Rank %d virtual_lane_id = %d, quota= %d, message length = %d, offset = %d\n", transport->rank, virtual_lane_id, arbitration_table[index].weight * BASE_UNIT_SIZE, message->length, message->current_offset); 
                        nbytes = arbitration_table[index].weight * BASE_UNIT_SIZE;

                        if (message->current_offset + nbytes >= message->length) {
                            nbytes = message->length - message->current_offset;
                            virtual_lanes[i].requests.erase(virtual_lanes[i].requests.begin());
                            //printf("Remove a message\n");
                        } else {
                            /*Update the virtual lane*/
                            virtual_lanes[i].requests.front()->current_offset += nbytes;
                        }

			struct optiq_message *instant = get_send_message(&transport->avail_send_messages);
			instant->buffer = &message->buffer[message->current_offset];
			instant->length = nbytes;
                        instant->current_offset = 0;
                        instant->next_dest = message->next_dest;
                        instant->source = transport->rank;
                        instant->header = message->header;

                        optiq_transport_send(transport, instant);

                        done = false;

                        //printf("Rank %d update new offset = %d\n", transport->rank, message->current_offset);

                        /*After process any queue, go back to the arbitration table*/
                        break;
                    }
                }
            }
        }
    }
}

void create_virtual_lane_arbitration_table(vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table, vector<struct optiq_job> &jobs, int world_rank)
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

                    arbitration_table.push_back(ab);
                    virtual_lanes.push_back(vl);
                }
            }
        }
    }
}

void add_message_to_virtual_lanes(struct optiq_message *message, vector<struct optiq_virtual_lane> *virtual_lanes)
{
    for (int i = 0; i < (*virtual_lanes).size(); i++) {
        if (message->header.flow_id == (*virtual_lanes)[i].id) {
            (*virtual_lanes)[i].requests.push_back(message);
        }
    }
}

void add_job_to_virtual_lanes(struct optiq_job &job, vector<struct optiq_virtual_lane> *virtual_lanes)
{
    char *buffer = (char *)job.buffer;
    int data_size = job.demand;

    int total_local_throughput = 0;

    for (int i = 0; i < job.flows.size(); i++) {
        /*Compute the total flows for the local node*/
        total_local_throughput += job.flows[i].throughput;
    }

    int num_virtual_lanes = (*virtual_lanes).size();

    /*Fill in the virtual lanes with data from local jobs*/
    int global_offset = 0, length = 0;
    for (int i = 0; i < job.flows.size(); i++) {
        length = ((double)job.flows[i].throughput / (double)total_local_throughput) * (double)data_size;
        struct optiq_message *message = (struct optiq_message *)core_memory_alloc(sizeof(struct optiq_message), "message", "add_job_to_virtual_lanes");
        message->header.original_length = data_size;
        message->header.original_offset = global_offset;
	message->header.job_id = job.id;
        message->header.flow_id = job.flows[i].id;
        message->header.final_dest = job.dest;
        message->header.original_source = job.source;
        message->next_dest = get_next_dest_from_flow(&job.flows[i], job.source);
        message->source = job.source;
        message->current_offset = 0;
        message->service_level = 0;
        message->buffer = &buffer[global_offset];
        message->length = length;
        global_offset += length;

        add_message_to_virtual_lanes(message, virtual_lanes);
        job.flows[i].message = message;
        job.flows[i].sent_bytes = 0;
    }
}
