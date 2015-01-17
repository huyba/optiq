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


void adding_load_on_path(struct path *np, int **load, int adding_load)
{
    for (int i = 0; i < np->arcs.size(); i++) {
	load[np->arcs[i].u][np->arcs[i].v] += adding_load;
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

void update_max_load(struct path *np, int **load, struct multibfs *bfs)
{
    int u = 0, v = 0;
    struct path *p;
    for (int k = 0; k < np->arcs.size(); k++) {
	u = np->arcs[k].u;
	v = np->arcs[k].v;

        for (int i = 0; i < bfs->edge_path[u][v].size(); i++) {
	    p = bfs->edge_path[u][v][i];
	    if (p->max_load < load[u][v]) {
		p->max_load = load[u][v];
		hp_shift_down(bfs->heap, p->hpos);
	    }
	}
    }
}

void adding_edge_path(std::vector<struct path*> **edge_path, struct path *p)
{
    for (int i = 0; i < p->arcs.size(); i++) {
	//printf("edge = %d #arcs = %d u = %d v = %d\n", edge_path[p->arcs[i].u][p->arcs[i].v].size(), p->arcs.size(), p->arcs[i].u, p->arcs[i].v);
	edge_path[p->arcs[i].u][p->arcs[i].v].push_back(p);
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

void build_paths(std::vector<struct path *> &complete_paths, int num_dests, int *dests, struct multibfs *bfs) 
{
    int num_dims = bfs->num_dims;
    int *size = bfs->size;
    int **graph = bfs->graph;

    int **load = bfs->load;
    bool ** visited = bfs->visited;

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }

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

		    struct path *p = (struct path *) calloc (1, sizeof(struct path));
		    p->arcs.push_back(a);
		    p->max_load = 1;
		    p->dest_id = i;

		    adding_edge_path(bfs->edge_path, p);
		    //printf("added edge path\n");
		    adding_load_on_path(p, load, 1);
		    //printf("added load\n");
		    hp_insert(bfs->heap, p);
		    //printf("inserted to heap\n");
		    complete_paths.push_back(p);
		    //printf("done adding %d %d\n", a.u, a.v);
		    
		    visited[i][j] = true;
		    done = false;
		    break;
		}
	    }
	}
    }

    //optiq_path_print_paths(complete_paths);
    
    /*for (int i = 0; i < num_nodes; i++) {
	for (int j = 0; j < num_nodes; j++) {
	    if (load[i][j] != 0) {
		printf("load[%d][%d] = %d\n", i, j, load[i][j]);
	    }
	}
    }*/

    //printf("Done first step size of complete_paths %d\n", complete_paths.size());
    
    hp_print(bfs->heap);

    gettimeofday(&t2, NULL);

    /*For the current number of paths, start expanding and add more path*/
    long int utime = 0L;
    int index;
    while(bfs->heap->num_elements != 0) {
	struct path *p = hp_find_min(bfs->heap);
	optiq_path_print_path(p);
	hp_remove_min(bfs->heap);

	struct arc a = p->arcs.back();
	int furthest_point = a.v;

	/*Expanding all paths from this point*/
	for (int i = 0; i < num_nodes; i++) {
	    if (graph[furthest_point][i] == 1 && !visited[p->dest_id][i]) {
		struct path *np = (struct path *) calloc (1, sizeof(struct path));
		np->arcs = p->arcs;
		np->max_load = p->max_load;
		np->dest_id = p->dest_id;

		struct arc na;
		na.u = a.v;
		na.v = i;
		np->arcs.push_back(na);
		adding_load_on_path(np, load, 1);
		adding_edge_path(bfs->edge_path, np);
		hp_insert(bfs->heap, np);
		complete_paths.push_back(np);

		gettimeofday(&t4, NULL);
		update_max_load(np, load, bfs);
		gettimeofday(&t5, NULL);
		long int diff = (t5.tv_usec + 1000000 * t5.tv_sec) - (t4.tv_usec + 1000000 * t4.tv_sec);
		utime += diff;

		visited[p->dest_id][i] = true;

		//optiq_path_print_path(np);
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
