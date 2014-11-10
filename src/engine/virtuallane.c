#include "virtuallane.h"

void print_arbitration_table(vector<struct optiq_arbitration> ab)
{
    int num_entries = ab.size();
    printf("Arbitration table: #entries = %d\n", num_entries);
    printf("vl_id weight priority\n");
    for (int i = 0; i < num_entries; i++) {
        printf("%d %d %d\n", ab[i].virtual_lane_id, ab[i].weight, ab[i].priority);
    }
}

void print_virtual_lanes(vector<struct optiq_virtual_lane> virtual_lanes)
{
    int num_virtual_lanes = virtual_lanes.size();

    printf("Current status of virtual lanes: \n");
    printf("Number of virtual lanes = %d\n", num_virtual_lanes);
    for (int i = 0; i < num_virtual_lanes; i++) {
        printf("Virtual lane id = %d, #messages = %lu\n", virtual_lanes[i].id, virtual_lanes[i].requests.size());

        for (int j = 0; j < virtual_lanes[i].requests.size(); j++) {
            printf("Message has %d bytes\n", virtual_lanes[i].requests[j].length);
        }
    }
}

void print_jobs(struct optiq_job *jobs, int num_jobs)
{
    printf("num_jobs = %d\n", num_jobs);

    struct optiq_flow *flow = NULL;

    for (int i = 0; i < num_jobs; i++) {
        printf("\njob_id = %d, source = %d , dest = %d, num_flows = %d\n", jobs[i].id, jobs[i].source, jobs[i].dest, jobs[i].num_flows);

        for (int j = 0; j < jobs[i].num_flows; j++) {
            flow = jobs[i].flows[j];

            printf("flow_id = %d, throughput = %d, num_arcs = %d\n", flow->id, flow->throughput, flow->num_arcs);
            for (int k = flow->num_arcs-1; k >= 0; k--) {
                printf("%d -> ", flow->arcs[k].ep1);
            }
            printf("%d\n", flow->arcs[0].ep2);
        }
    }
}

void transfer_from_virtual_lanes(const vector<struct optiq_arbitration> arbitration_table, vector<struct optiq_virtual_lane> virtual_lanes)
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
                        struct optiq_message message = virtual_lanes[i].requests.front();

                        printf("virtual_lane_id = %d, quota= %d\n", virtual_lane_id, arbitration_table[index].weight * BASE_UNIT_SIZE);
                        printf(" message length = %d, offset = %d\n", message.length, message.current_offset);

                        nbytes = arbitration_table[index].weight * BASE_UNIT_SIZE;

                        if (message.current_offset + nbytes >= message.length) {
                            nbytes = message.length - message.current_offset;
                            virtual_lanes[i].requests.erase(virtual_lanes[i].requests.begin());
                            printf("Remove a message\n");
                        } else {
                            /*Update the virtual lane*/
                            virtual_lanes[i].requests.front().current_offset += nbytes;
                        }

                        optiq_send((void *)&message.buffer[message.current_offset], nbytes, message.dest, message.flow_id);
                        done = false;

                        printf("offset = %d\n", message.current_offset);

                        /*After process any queue, go back to the arbitration table*/
                        break;
                    }
                }
            }
        }
    }
}

void create_virtual_lane_arbitration_table(vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table, int num_jobs, const struct optiq_job *jobs, int world_rank)
{
    struct optiq_flow *flow = NULL;

    for (int i = 0; i < num_jobs; i++) {
        for (int j = 0; j < jobs[i].num_flows; j++) {
            flow = jobs[i].flows[j];
            for (int k = flow->num_arcs-1; k >= 0; k--) {
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

void add_message_to_virtual_lanes(char *buffer, int data_size, const optiq_job &job, vector<struct optiq_virtual_lane> &virtual_lanes)
{
    int total_local_throughput = 0;

    for (int i = 0; i < job.num_flows; i++) {
        /*Compute the total flows for the local node*/
        total_local_throughput += job.flows[i]->throughput;
    }

    int num_virtual_lanes = virtual_lanes.size();

    /*Fill in the virtual lanes with data from local jobs*/
    int global_offset = 0, length = 0;
    for (int i = 0; i < job.num_flows; i++) {
        length = ((double)job.flows[i]->throughput / (double)total_local_throughput) * (double)data_size;
        struct optiq_message message;
        message.job_id = job.id;
        message.flow_id = job.flows[i]->id;
        message.dest = job.dest;
        message.current_offset = 0;
        message.service_level = 0;
        message.buffer = &buffer[global_offset];
        message.length = length;
        global_offset += length;

        for (int j = 0; j < num_virtual_lanes; j++) {
            if (message.flow_id == virtual_lanes[j].id) {
                virtual_lanes[j].requests.push_back(message);
            }
        }
    }
}


