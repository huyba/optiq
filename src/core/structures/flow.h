#ifndef OPTIQ_FLOW
#define OPTIQ_FLOW

#include <vector>

#include "../../engine/message.h"

using namespace std;

struct optiq_arc {
    int ep1;
    int ep2;
    int capacity;
};

struct optiq_flow {
    int id;
    int throughput;
    vector<struct optiq_arc> arcs;
    struct optiq_message *message;
    int registered_bytes;
    int sent_bytes;
};

int get_next_dest_from_flow(const struct optiq_flow *flow, int current_ep);

#endif
