#ifndef OPTIQ_GRAPH_BGQ_H
#define OPTIQ_GRAPH_BGQ_H

#include <stdlib.h>
#include <vector>

using namespace std;

struct optiq_supernode {
    int id;
    vector<int> node_ids;
};

struct optiq_bgq_node {
    int id;
    int coord[5];
    int color;
};

int** optiq_graph_build_nodes_graph_bgq(int *size, vector<struct optiq_bgq_node *> *nodes);

int optiq_graph_coarsen_bgq(vector<struct optiq_supernode> *supernodes, vector<struct optiq_arc> *superarcs, map<int, int> *node_supernode);

#endif
