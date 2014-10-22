#ifndef OPTIQ_GRAPH_INTERFACE_H
#define OPTIQ_GRAPH_INTERFACE_H

struct graph_info {
    int num_nodes;
    optiq_node *nodes;
    float **capacity;
    float **arc;   
};

struct graph_interface {
    void (*optiq_graph_init)(struct topology_info *topo);
    void (*optiq_graph_construct)(struct topology_info *topo, optiq_graph *graph);
    void (*optiq_graph_coarsen)(struct optiq_graph *graph);
    void (*optiq_graph_uncoarsen)(struct optiq_graph *graph);
};

#endif
