#ifndef OPTIQ_FLOW
#define OPTIQ_FLOW

struct flow {
    int source;
    int destination;
    float demand;
};

void read_flows_from_file(char *filePath);

#endif
