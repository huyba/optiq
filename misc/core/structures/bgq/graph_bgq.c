#include <stdlib.h>
#include <vector>
#include <map>
#include <queue>

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

void optiq_graph_add_superarc(int u_sp, int v_sp, vector<struct optiq_arc *> *superarcs, int capacity)
{
#ifdef DEBUG_BUILD_GRAPH_BGQ
    printf("u = %d, v = %d, capacity = %d, superarcs.size = %ld\n", u_sp, v_sp, capacity, superarcs->size());
#endif

    /*Check to see if exist*/
    bool existing = false;
    for (int i = 0; i < superarcs->size(); i++) {
	if (superarcs->at(i)->ep1 == u_sp && superarcs->at(i)->ep2 == v_sp) {
	    superarcs->at(i)->capacity += capacity;
	    existing = true;
	    break;
	}
    }

    if (!existing) {
	struct optiq_arc *arc = (struct optiq_arc *)malloc(sizeof(struct optiq_arc));
	arc->ep1 = u_sp;
	arc->ep2 = v_sp;
	arc->capacity = capacity;
	superarcs->push_back(arc);
    }
}

int optiq_graph_coarsen_bgq(int **graph, int num_nodes, vector<struct optiq_supernode *> *supernodes, vector<struct optiq_arc *> *superarcs, std::map<int, int> *node_supernode)
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

        supernodes->push_back(supernode);
    }

    /*Create new superarcs from arcs*/
    std::queue<int> queue;
    queue.push(0);
    int u, v, u_sp, v_sp;
    std::map<int, int>::iterator iter;
    int capacity = 2048;

    while(!queue.empty()) {
	u = queue.front();
	queue.pop();

	for (v = 0; v < num_nodes; v++) {
	    if (graph[u][v] > 0) {
		queue.push(v);
		graph[u][v] = 0;

		/*Get the supernode_ids of both nodes to see if they are in different supernodes*/
		iter = node_supernode->find(u);
		if (iter != node_supernode->end()) {
		    u_sp = iter->second;
		}
		iter = node_supernode->find(v);
		if (iter != node_supernode->end()) {
		    v_sp = iter->second;
		}

#ifdef DEBUG_BUILD_GRAPH_BGQ
		printf("u = %d, u_sp = %d, v = %d, v_sp = %d\n", u, u_sp, v, v_sp);
#endif

		if (u_sp != v_sp) {
		    optiq_graph_add_superarc(u_sp, v_sp, superarcs, capacity);
		}
	    }
	}
    }

    return 0;
}
