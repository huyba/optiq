#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <vector>

#include "util.h"
#include "datagen.h"
#include "topology.h"

#include <sys/time.h>

#include "multibfs.h"

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

void optiq_path_print_stat(std::vector<struct path> &paths, int num_nodes)
{
    int max_hops = 0;
    int max_load = 0;

    int **load = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
	load[i]  = (int *)malloc(sizeof(int) * num_nodes);
	memset(load[i], 0, num_nodes * sizeof(int));
    }

    for (int i = 0; i < paths.size(); i++) 
    {
	struct path p = paths[i];

	if (max_hops < p.arcs.size()) {
            max_hops = p.arcs.size();
        }

	for (int j = 0; j < p.arcs.size(); j++) {
	    load[p.arcs[j].u][p.arcs[j].v]++;
	}
    }

    for (int i = 0; i < num_nodes; i++) {
	for (int j = 0; j < num_nodes; j++) {
	    if (load[i][j] != 0) {
		printf("load %d %d %d\n", i, j, load[i][j]);

		if (max_load < load[i][j]) {
		    max_load = load[i][j];
		}
	    }
	}
    }

    printf("max_hop = %d\n", max_hops);
    printf("max_load = %d\n", max_load);
}

void optiq_path_print_paths(std::vector<struct path> &paths)
{
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) {
        struct path p = paths[i];

	printf("path %d num_hops = %ld\n", i, p.arcs.size());

	printf("path %d: ", i);
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
	/*printf("path %d load %d\n", i, paths[i].max_load);*/
	if (min_load > paths[i].max_load) {
	    min_load = paths[i].max_load;
	    min_hops = paths[i].arcs.size();
	    index = i;
	} else if (min_load == paths[i].max_load && min_hops > paths[i].arcs.size()) {
	    min_hops = paths[i].arcs.size();
	    index = i;
	}
    }

    /*printf("index = %d\n\n", index);*/

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

void multibfs_init(struct multibfs *bfs)
{
    int num_dims = bfs->num_dims;
    int *size = bfs->size;
    int **all_coords = NULL;
    int **graph = NULL;
    int **load = NULL;
    bool ** visited = NULL;

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
        num_nodes *= size[i];
    }
    

    all_coords = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        all_coords[i] = (int *)malloc(sizeof(int) * num_dims);
        for (int j = 0; j < num_dims; j++) {
            all_coords[i][j] = 0;
        }
    }

    graph = (int **)malloc(sizeof(int *) * num_nodes);
    load = (int **)malloc(sizeof(int *) * num_nodes);
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

    visited = (bool **)malloc(sizeof(bool *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
        visited[i] = (bool *)malloc(sizeof(bool) * num_nodes);
        for (int j = 0; j < num_nodes; j++) {
            visited[i][j] = false;
        }
    }

    bfs->all_coords = all_coords;
    bfs->graph = graph;
    bfs->load = load;
    bfs->visited = visited;
}

void build_paths(std::vector<struct path> &complete_paths, int num_dests, int *dests, struct multibfs *bfs) 
{
    int num_dims = bfs->num_dims;
    int *size = bfs->size;
    int **graph = bfs->graph;

    int **load = bfs->load;
    bool ** visited = bfs->visited;

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
	printf("%d ", size[i]);
    }
    printf("num_nodes %d\n", num_nodes);

    /*Clean the arrays*/
    for (int i = 0; i < num_dests; i++) {
	memset(visited[i], 0, sizeof(bool) * num_nodes);
    }

    for (int i = 0; i < num_nodes; i++) {
        memset(load[i], 0, sizeof(int) * num_nodes);
    }

    std::vector<struct path> expanding_paths;
    
    struct timeval t1, t2, t3, t4, t5;

    gettimeofday(&t1, NULL);

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

    gettimeofday(&t2, NULL);

    /*For the current number of paths, start expanding and add more path*/
    long int utime = 0L;
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

		gettimeofday(&t4, NULL);
		update_max_load(np, load, expanding_paths);
		gettimeofday(&t5, NULL);
		long int diff = (t5.tv_usec + 1000000 * t5.tv_sec) - (t4.tv_usec + 1000000 * t4.tv_sec);
		utime += diff;

		visited[p.dest_id][i] = true;
	    }
	}
    }

    gettimeofday(&t3, NULL);

    long int diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);

    printf("Extend 2 in %ld microseconds\n", diff);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Extend 1 %ld microseconds\n", diff);
    printf("Total update time is %ld\n", utime);
}


void naive_build_paths(std::vector<struct path> &complete_paths, int num_dests, int *dests, struct multibfs *bfs) 
{
    int num_dims = bfs->num_dims;
    int *size = bfs->size;
    int **graph = bfs->graph;

    int **load = bfs->load;
    bool ** visited = bfs->visited;

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
	printf("%d ", size[i]);
    }
    printf("num_nodes %d\n", num_nodes);

    /*Clean the arrays*/
    for (int i = 0; i < num_dests; i++) {
	memset(visited[i], 0, sizeof(bool) * num_nodes);
    }

    for (int i = 0; i < num_nodes; i++) {
        memset(load[i], 0, sizeof(int) * num_nodes);
    }

    std::vector<struct path> expanding_paths;
    
    struct timeval t1, t2, t3;

    gettimeofday(&t1, NULL);

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

    gettimeofday(&t2, NULL);

    /*For the current number of paths, start expanding and add more path*/
    long int utime = 0L;

    while (!expanding_paths.empty()) {
	struct path p = expanding_paths.front();
	expanding_paths.erase(expanding_paths.begin());

	struct arc a = p.arcs.back();
	int furthest_point = a.v;

	/*Expanding all paths from this point*/
	for (int i = 0; i < num_nodes; i++) {
	    if (graph[furthest_point][i] == 1 && !visited[p.dest_id][i]) {
		struct path np = p;
		struct arc na;
		na.u = a.v;
		na.v = i;
		np.arcs.push_back(na);
		expanding_paths.push_back(np);
		complete_paths.push_back(np);
		visited[p.dest_id][i] = true;
	    }
	}
    }

    gettimeofday(&t3, NULL);

    long int diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);

    printf("Extend 2 in %ld microseconds\n", diff);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);

    printf("Extend 1 %ld microseconds\n", diff);
    printf("Total update time is %ld\n", utime);
}
