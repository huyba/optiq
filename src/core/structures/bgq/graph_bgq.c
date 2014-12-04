#include <stdlib.h>
#include <vector>
#include <map>

#include "../system/bgq/topology_bgq.h"
#include "../system/memory.h"
#include "graph_bgq.h"


int** optiq_graph_build_nodes_graph_bgq(int *size, vector<struct optiq_bgq_node *> *nodes)
{
    int num_dims = 5;
    int coord[5];
    int nid;
    int neighbors[10];
    int num_neighbors = 0;

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
        num_nodes *= size[i];
    }

    int **graph = (int **)malloc(sizeof(int *) * num_nodes);

    for (int i = 0; i < num_nodes; i++) {
        graph[i] = (int *)malloc(sizeof(int) * num_nodes);
        for (int j = 0; j < num_nodes; j++) {
            graph[i][j] = 0;
        }
    }

    for (int ad = 0; ad < size[0]; ad++) {
        coord[0] = ad;
        for (int bd = 0; bd < size[1]; bd++) {
            coord[1] = bd;
            for (int cd = 0; cd < size[2]; cd++) {
                coord[2] = cd;
                for (int dd = 0; dd < size[3]; dd++) {
                    coord[3] = dd;
                    for (int ed = 0; ed < size[4]; ed++) {
                        coord[4] = ed;
                        num_neighbors = 0;
                        optiq_topology_get_node_id_from_coord_bgq(num_dims, size, coord, &nid);

                        struct optiq_bgq_node *node = (struct optiq_bgq_node*)malloc(sizeof(struct optiq_bgq_node));
                        node->id = nid;
                        for (int i = 0; i < num_dims; i++) {
                            node->coord[i] = coord[i];
                        }
                        nodes->push_back(node);

                        optiq_topology_get_neighbor_ids_bgq(num_dims, size, coord, neighbors, &num_neighbors);
                        for (int i = 0; i < num_neighbors; i++) {
                            graph[nid][neighbors[i]] = 1;
                        }
                    }
                }
            }
        }
    }

    return graph;
}

int optiq_graph_coarsen_bgq(vector<struct optiq_supernode> *supernodes, vector<struct optiq_arc> *superarcs, map<int, int> *node_supernode)
{
    int num_supernodes = 8;
    int nodes_per_supernode = 32;

    int node_id = 0;

    /*Create new supernodes from node and a mapping between them*/
    for (int i = 0; i < num_supernodes; i++) {
        struct optiq_supernode *supernode = (struct optiq_supernode *)core_memory_alloc(sizeof(struct optiq_supernode), "supernode", "optiq_graph_coarse");
        supernode->id = i;

        for (int j = 0; j < nodes_per_supernode; j++) {
            node_id = j + i * nodes_per_supernode;
            supernode->node_ids.push_back(node_id);
            node_supernode->insert(make_pair(node_id, supernode->id));
        }

        (*supernodes).push_back(*supernode);
    }

    /*Create new superarcs from arcs*/
    int neighbors[9];
    int num_neighbors;
    int neighbor_id;
    int neighbor_sp_id;
    std::map<int, int>::iterator it;

    for (int i = 0; i < supernodes->size(); i++) {
        for (int j = 0; j < supernodes->at(i).node_ids.size(); j++) {
            node_id = supernodes->at(i).node_ids[j];

            /*Get neighbors of the current node*/
            get_neighbors(node_id, neighbors, &num_neighbors);

            /*For each neighbor, determine if an superarc exists*/
            for (int k = 0; k < num_neighbors; k++) {
                neighbor_id = neighbors[k];

                /*Get the supernode id of each neighbor*/
                it = node_supernode->find(neighbor_id);
                if (it != node_supernode->end()) {
                    neighbor_sp_id = it->second();

                    /*If they belong to 2 different supernodes*/
                    if (neighbor_sp_id != supernodes->at(i).id) {
                        add_super_arc(node_id, supernodes->at(i).id, neighbor_id, neighbor_sp_id);
                    }
                }
            }
        }
    }


    return 0;
}
