#ifndef OPTIQ_VIRTUAL_LANE
#define OPTIQ_VIRTUAL_LANE

#include <vector>
#include <string>

#include "../core/structures/flow.h"
#include "transport/transport.h"

#define BASE_UNIT_SIZE 1024

using namespace std;

struct optiq_message_header {
    int final_dest;
    int flow_id;
    int original_offset;
    int original_length;
};

struct optiq_message {
    char *buffer;
    int next_dest;
    int service_level;
    int length;
    int current_offset;
    struct optiq_message_header header;
};

struct optiq_virtual_lane {
    int id;
    vector<struct optiq_message> requests;
};

struct optiq_arbitration {
    int virtual_lane_id;
    int weight;
    int priority;
};

void print_arbitration_table(vector<struct optiq_arbitration> ab);
void print_virtual_lanes(vector<struct optiq_virtual_lane> virtual_lanes);
void transport_from_virtual_lanes(struct optiq_transport *transport, const vector<struct optiq_arbitration> arbitration_table, vector<struct optiq_virtual_lane> virtual_lanes);
void create_virtual_lane_arbitration_table(vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table, int num_jobs, const struct optiq_job *jobs, int world_rank);
void add_message_to_virtual_lanes(char *buffer, int data_size, const optiq_job &job, vector<struct optiq_virtual_lane> &virtual_lanes);


#endif
