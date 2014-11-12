#include "job.h"

void add_job_to_virtual_lanes(const optiq_job &job, vector<struct optiq_virtual_lane> &virtual_lanes)
{
    char *buffer = (char *)job.buffer;
    int data_size = job.demand;

    int total_local_throughput = 0;

    for (int i = 0; i < job.num_flows; i++) {
        /*Compute the total flows for the local node*/
        total_local_throughput += job.flows[i].throughput;
    }

    int num_virtual_lanes = virtual_lanes.size();

    /*Fill in the virtual lanes with data from local jobs*/
    int global_offset = 0, length = 0;
    for (int i = 0; i < job.num_flows; i++) {
        length = ((double)job.flows[i].throughput / (double)total_local_throughput) * (double)data_size;
        struct optiq_message message;
        message.header.original_length = data_size;
        message.header.original_offset = global_offset;
        message.header.flow_id = job.flows[i].id;
        message.header.final_dest = job.dest;
        message.next_dest = get_next_dest_from_flow(job.flows[i], job.source);
        message.current_offset = 0;
        message.service_level = 0;
        message.buffer = &buffer[global_offset];
        message.length = length;
        global_offset += length;

        for (int j = 0; j < num_virtual_lanes; j++) {
            if (message.header.flow_id == virtual_lanes[j].id) {
                virtual_lanes[j].requests.push_back(message);
            }
        }
    }
}
