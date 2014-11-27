#ifndef OPTIQ_VIRTUAL_LANE
#define OPTIQ_VIRTUAL_LANE

#include <vector>
#include <map>
#include <string>

#include "../core/structures/job.h"
#include "transport/transport.h"
#include "message.h"

#define BASE_UNIT_SIZE 1024

using namespace std;

struct optiq_arbitration {
    int virtual_lane_id;
    int weight;
    int priority;
};

struct optiq_virtual_lane {
    int id;
    vector<struct optiq_message *> requests;
};

struct optiq_vlab {
    map<int, struct optiq_virtual_lane> vl;
    vector<struct optiq_arbitration> ab;
};

void print_arbitration_table(vector<struct optiq_arbitration> &ab);
void print_virtual_lanes(map<int, struct optiq_virtual_lane> &virtual_lanes);

int optiq_vlab_create(struct optiq_vlab &vlab, vector<struct optiq_job> &jobs, int world_rank);
int optiq_vlab_add_message(struct optiq_vlab &vlab, struct optiq_message *message);
int optiq_vlab_add_job(struct optiq_vlab &vlab, struct optiq_job &job, struct optiq_transport *transport);
void optiq_vlab_transport(struct optiq_vlab &vlab, struct optiq_transport *transport);

#endif
