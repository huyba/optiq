#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "datagen.h"
#include "topology.h"

#include <vector>

struct arc {
    int u;
    int v;
};

struct path {
    int max_load;
    int dest_id;
    std::vector<struct arc> arcs;
};

void optiq_neighbor_print_neighbor(int node_id, int **graph, int num_nodes)
{
    printf("Node_id = %d. Neighbors = ", node_id);
    for (int i = 0; i < num_nodes; i++) {
	if (graph[node_id][i] == 1) {
	    printf("%d ", i);
	}
    }
    printf("\n");
}


void optiq_path_print_paths(std::vector<struct path> paths)
{
    for (int i = 0; i < paths.size(); i++) {
        struct path p = paths[i];
        for (int j = 0; j < p.arcs.size(); j++) {
            printf("%d->", p.arcs[j].u);
        }
        printf("%d\n", p.arcs.back().v);
    }
}

int main(int argc, char **argv) 
{
    int num_dims = 5;
    int size[5] = {2, 4, 4, 4, 2};
    //int torus[5] = {0, 1, 1, 1, 1};
    int order[5] = {0, 0, 0, 0, 0};

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }
    
    int **all_coords = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
	all_coords[i] = (int *)malloc(sizeof(int) * num_dims);
	for (int j = 0; j < num_dims; j++) {
	    all_coords[i][j] = 0;
	}
    }

    int **graph = (int **)malloc(sizeof(int *) * num_nodes);
    int **load = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        graph[i] = (int *)malloc(sizeof(int) * num_nodes);
	load[i] = (int *)malloc(sizeof(int) * num_nodes);
        for (int j = 0; j < num_nodes; j++) {
            graph[i][j] = 0;
	    load[i][j] = 0;
        }
    }

    int coord[5], nid = 0, neighbors[10], num_neighbors = 0;
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
			nid = optiq_compute_nid(num_dims, size, coord);
			for (int i = 0; i < num_dims; i++) {
			    all_coords[nid][i] = coord[i];
			}

			num_neighbors = optiq_compute_neighbors(num_dims, coord, size, neighbors);
                        for(int i = 0; i < num_neighbors; i++) {
                            graph[nid][neighbors[i]] = 1;
                        }
		    }
		}
	    }
        }
    }

    optiq_neighbor_print_neighbor(32, graph, num_nodes);

    optiq_topology_compute_routing_order_bgq(num_dims, size, order);

    printf("Order [%d %d %d %d %d]\n", order[0], order[1], order[2], order[3], order[4]);

    int num_dests = 4;
    int dests[4] = {32, 96, 160, 224};

    bool **visited = (bool **)malloc(sizeof(bool *) * num_dests);
    for (int i = 0; i < num_dests; i++) {
	visited[i] = (bool *)malloc(sizeof(bool) * num_nodes);
	for (int j = 0; j < num_nodes; j++) {
	    visited[i][j] = false;
	}
    }

    std::vector<struct path> complete_paths;
    std::vector<struct path> expanding_paths;
    
    /*Adding outgoing paths of the destination nodes first*/
    bool done = false;
    while(!done) {
	done = true;
	for (int i = 0; i < num_dests; i++) {
	    for (int j = 0; j < num_nodes; j++) {
		if ((graph[dests[i]][j] == 1) && !visited[i][j]) {
		    struct arc a;
		    a.u = dests[i];
		    a.v = j;
		    load[dests[i]][j]++;
		    struct path p;
		    p.arcs.push_back(a);
		    p.max_load = 1;
		    p.dest_id = i;
		    expanding_paths.push_back(p);
		    complete_paths.push_back(p);
		    visited[i][j] = true;
		    done = false;
		    break;
		}	
	    }
	}
    }

    std::vector<struct path>::iterator iter;

    /*For the current number of paths, start expanding and add more path*/
    while(!expanding_paths.empty()) {
	struct path p = expanding_paths.back();
	expanding_paths.pop_back();

	struct arc a = p.arcs.back();
	int furthest_point = a.v;

	/*Expanding all paths from this point*/
	for (int i = 0; i < num_nodes; i++) {
	    if (graph[furthest_point][i] == 1 && !visited[p.dest_id][i]) {
		struct path np = p;
		struct arc na;
		na.u = a.v;
		na.v = i;
		load[a.v][i]++;
		np.arcs.push_back(na);
		expanding_paths.push_back(np);
		complete_paths.push_back(np);
		visited[p.dest_id][i] = true;
	    }
	}
    }

    int max_load = 0, u, v;
    for (int i = 0; i < num_nodes; i++) {
	for (int j = 0; j < num_nodes; j++) {
	    if (max_load < load[i][j]) {
		max_load = load[i][j];
		u = i; v = j;
	    }
	}
    }

    optiq_path_print_paths(complete_paths);

    printf("[%d, %d] = %d\n", u, v, max_load);

}
