#ifndef OPTIQ_VIRTUAL_LANE
#define OPTIQ_VIRTUAL_LANE

#include <vector>
#include <string>

#include "../core/structures/flow.h"

#define BASE_UNIT_SIZE 1024

using namespace std;

struct optiq_message {
    char *buffer;
    int dest;
    int service_level;
    int length;
    int current_offset;
    int job_id;
    int flow_id;
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
void print_jobs(struct optiq_job *jobs, int num_jobs);
void transfer_from_virtual_lanes(const vector<struct optiq_arbitration> arbitration_table, vector<struct optiq_virtual_lane> virtual_lanes);
void create_virtual_lane_arbitration_table(vector<struct optiq_virtual_lane> &virtual_lanes, vector<struct optiq_arbitration> &arbitration_table, int num_jobs, const struct optiq_job *jobs, int world_rank);
void add_message_to_virtual_lanes(char *buffer, int data_size, const optiq_job &job, vector<struct optiq_virtual_lane> &virtual_lanes);


#endif
