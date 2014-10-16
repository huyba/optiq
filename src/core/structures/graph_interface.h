#ifndef OPTIQ_GRAPH_INTERFACE_H
#define OPTIQ_GRAPH_INTERFACE_H

struct graph_interface {
    void (*optiq_graph_init)(struct topology_info *topo);
    void (*optiq_graph_construct)(struct topology_info *topo, float **graph);
}

#endif
