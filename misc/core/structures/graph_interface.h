#ifndef OPTIQ_GRAPH_INTERFACE_H
#define OPTIQ_GRAPH_INTERFACE_H

struct optiq_graph_info {
    int num_nodes;
    optiq_node *nodes;
    float **capacity;
    float **arc; 
};

struct optiq_graph_interface {
    void (*optiq_graph_init)(struct topology_info *topo, machine_type machine);
    void (*optiq_graph_construct)(struct topology_info *topo, struct optiq_graph_info *graph_info);
    void (*optiq_graph_coarsen)(struct optiq_graph_info *graph_info);
    void (*optiq_graph_uncoarsen)(struct optiq_graph_info *graph_info);
};

#endif
