#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <vector>

#include "util.h"
#include "datagen.h"
#include "topology.h"

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
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) {
        struct path p = paths[i];
        for (int j = 0; j < p.arcs.size(); j++) {
            printf("%d->", p.arcs[j].u);
        }
        printf("%d\n", p.arcs.back().v);
    }
}

void adding_load_on_path(struct path np, int **load, int adding_load)
{
    for (int i = 0; i < np.arcs.size(); i++) {
	load[np.arcs[i].u][np.arcs[i].v] += adding_load;
    }
}

int pick_up_path(std::vector<struct path> &paths)
{
    int min_load = INT_MAX;
    int min_hops = INT_MAX;
    int index = 0;

    for (int i = 0; i < paths.size(); i++) {
	printf("path %d load %d\n", i, paths[i].max_load);
	if (min_load > paths[i].max_load) {
	    min_load = paths[i].max_load;
	    min_hops = paths[i].arcs.size();
	    index = i;
	} else if (min_load == paths[i].max_load && min_hops > paths[i].arcs.size()) {
	    min_hops = paths[i].arcs.size();
	    index = i;
	}
    }

    printf("index = %d\n\n", index);

    return index;
}

void update_max_load(struct path &np, int **load, std::vector<struct path> &paths)
{
    int u = 0, v = 0;
    for (int k = 0; k < np.arcs.size(); k++) {
	u = np.arcs[k].u;
	v = np.arcs[k].v;

        for (int i = 0; i < paths.size(); i++) {
	   for (int j = 0; j < paths[i].arcs.size(); j++) {
		if (paths[i].arcs[j].u == u && paths[i].arcs[j].v == v && paths[i].max_load < load[u][v]) {
		    paths[i].max_load = load[u][v];
		    //printf("Updated maxload to %d of %d %d\n", load[u][v], u, v);
		}
	    }
	}
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

			num_neighbors = optiq_compute_neighbors(num_dims, size, coord, neighbors);
                        for(int i = 0; i < num_neighbors; i++) {
                            graph[nid][neighbors[i]] = 1;
                        }
		    }
		}
	    }
        }
    }

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
		    adding_load_on_path(p, load, 1);
		    expanding_paths.push_back(p);
		    complete_paths.push_back(p);
		    
		    visited[i][j] = true;
		    done = false;
		    break;
		}	
	    }
	}
    }

    /*For the current number of paths, start expanding and add more path*/
    int index;
    while(!expanding_paths.empty()) {
	index = pick_up_path(expanding_paths);
	struct path p = expanding_paths[index];
	expanding_paths.erase(expanding_paths.begin() + index);

	struct arc a = p.arcs.back();
	int furthest_point = a.v;

	/*Expanding all paths from this point*/
	for (int i = 0; i < num_nodes; i++) {
	    if (graph[furthest_point][i] == 1 && !visited[p.dest_id][i]) {
		struct path np = p;
		struct arc na;
		na.u = a.v;
		na.v = i;
		adding_load_on_path(np, load, 1);
		np.arcs.push_back(na);
		expanding_paths.push_back(np);
		complete_paths.push_back(np);
		update_max_load(np, load, complete_paths);
		visited[p.dest_id][i] = true;
	    }
	}
    }

    int max_load = 0, u, v;
    for (int i = 0; i < num_nodes; i++) {
	for (int j = 0; j < num_nodes; j++) {
	    if (graph[i][j] == 1) {
		printf("Load on [%d %d] %d\n", i, j, load[i][j]);
	    }
	    if (max_load < load[i][j]) {
		max_load = load[i][j];
		u = i; v = j;
	    }
	}
    }

    optiq_path_print_paths(complete_paths);

    printf("[%d, %d] = %d\n", u, v, max_load);

}
