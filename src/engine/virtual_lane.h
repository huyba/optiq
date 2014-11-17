#ifndef OPTIQ_VIRTUAL_LANE
#define OPTIQ_VIRTUAL_LANE

#include <vector>
#include <string>

#include "../core/structures/job.h"
#include "transport/transport.h"
#include "message.h"

#define BASE_UNIT_SIZE 1024

using namespace std;

struct optiq_virtual_lane {
    int id;
    vector<struct optiq_message *> requests;
};

struct optiq_arbitration {
    int virtual_lane_id;
    int weight;
    int priority;
};

void print_arbitration_table(vector<struct optiq_arbitration> ab);
void print_virtual_lanes(vector<struct optiq_virtual_lane> &virtual_lanes);
void transport_from_virtual_lanes(struct optiq_transport *transport, const vector<struct optiq_arbitration> arbitration_table, vector<struct optiq_virtual_lane> virtual_lanes);
void create_virtual_lane_arbitration_table(vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table, vector<struct optiq_job> &jobs, int world_rank);
void add_job_to_virtual_lanes(struct optiq_job &job, vector<struct optiq_virtual_lane> *virtual_lanes);
void add_message_to_virtual_lanes(struct optiq_message *message, vector<struct optiq_virtual_lane> *virtual_lanes);

#endif
