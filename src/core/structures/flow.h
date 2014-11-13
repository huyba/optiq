#ifndef OPTIQ_FLOW
#define OPTIQ_FLOW

#include <vector>

using namespace std;

struct optiq_arc {
    int ep1;
    int ep2;
};

struct optiq_flow {
    int id;
    int throughput;
    int num_arcs;
    vector<struct optiq_arc> arcs;
};

int get_next_dest_from_flow(const struct optiq_flow &flow, int current_ep);

#endif
