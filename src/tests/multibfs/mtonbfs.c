#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <vector>

#include "util.h"
#include "datagen.h"
#include "topology.h"

#include <sys/time.h>

#include "optiq_perf.h"
#include "mtonbfs.h"

struct multibfs_perf mperf;

void mton_add_load_on_path(struct path *np, int *load, int adding_load, int num_nodes)
{
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int i = 0; i < np->arcs.size(); i++) {
	load[np->arcs[i].u * num_nodes + np->arcs[i].v] += adding_load;
    }

    gettimeofday(&t1, NULL);
    long int diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    mperf.add_load_time += diff;
}

void mton_update_max_load(struct path *np, int *load, struct mtonbfs *bfs)
{
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    int u = 0, v = 0;
    struct path *p;
    for (int k = 0; k < np->arcs.size(); k++) {
	u = np->arcs[k].u;
	v = np->arcs[k].v;

        for (int i = 0; i < bfs->edge_path[u * bfs->num_nodes + v].size(); i++) {
	    p = bfs->edge_path[u * bfs->num_nodes + v][i];
	    if (p->max_load < load[u * bfs->num_nodes + v]) {
		p->max_load = load[u * bfs->num_nodes + v];
		hp_shift_down(bfs->heap, p->hpos);
	    }
	}
    }

    gettimeofday(&t1, NULL);
    long int diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    mperf.update_max_load_time += diff;
}

void mton_add_edge_path(std::vector<struct path*> *edge_path, struct path *p, int num_nodes)
{
    struct timeval t0, t1;
    gettimeofday(&t0, NULL);

    for (int i = 0; i < p->arcs.size(); i++) {
	//printf("edge = %d #arcs = %d u = %d v = %d\n", edge_path[p->arcs[i].u][p->arcs[i].v].size(), p->arcs.size(), p->arcs[i].u, p->arcs[i].v);
	edge_path[p->arcs[i].u * num_nodes + p->arcs[i].v].push_back(p);
    }

    gettimeofday(&t1, NULL);
    long int diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    mperf.add_edge_path_time += diff;
}

void mton_build_paths(std::vector<struct path *> &complete_paths, int num_sources, int *source_ranks, int num_dests, int *dest_ranks, struct mtonbfs *bfs) 
{
    int num_dims = bfs->num_dims;
    int num_nodes = bfs->num_nodes;

    std::vector<struct path> expanding_paths;

    struct timeval t0, t1, t2, t3;

    gettimeofday(&t0, NULL);

    int *source_dest = (int *)calloc (1, sizeof(int) * num_nodes * num_nodes);
    for (int i = 0; i < num_sources; i++) {
	for (int j = 0; j < num_dests; j++) {
	    source_dest[i * num_nodes + j] = 1;
	}
    }

    int *load = (int *)calloc(1, sizeof(int) * num_nodes * num_nodes);
    bool *visited = (bool *)calloc(1, sizeof(bool) * num_sources * num_nodes);

    bfs->paths = (struct path *) calloc (1, sizeof (struct path) * num_sources * num_nodes);
    int max_avail_path_id = 0;

    bfs->edge_path = (std::vector<struct path *> *) calloc (1, sizeof(std::vector<struct path *>) * num_nodes * num_nodes);

    /*Create a heap of paths*/
    bfs->heap = (struct heap_path *) malloc (sizeof (struct heap_path));
    hp_create(bfs->heap, num_sources * num_nodes);

    gettimeofday(&t1, NULL);

    /*Adding outgoing paths of the destination nodes first*/
    bool done = false;
    int neighbor_rank;

    while (!done) {
	done = true;
	for (int i = 0; i < num_sources; i++) 
	{
	    for (int j = 0; j < bfs->neighbors[source_ranks[i]].size(); j++) 
	    {
		neighbor_rank = bfs->neighbors[source_ranks[i]][j];
		if (!visited[i * num_nodes + neighbor_rank]) 
		{
		    struct arc a;
		    a.u = source_ranks[i];
		    a.v = neighbor_rank;

		    struct path *p = &bfs->paths[max_avail_path_id];
		    max_avail_path_id++;

		    p->arcs.push_back(a);
		    p->max_load = 1;
		    p->root_id = i;

		    mton_add_edge_path(bfs->edge_path, p, num_nodes);
		    //printf("added edge path\n");
		    mton_add_load_on_path(p, load, 1, num_nodes);
		    //printf("added load\n");
		    hp_insert(bfs->heap, p);
		    //printf("inserted to heap\n");
		    if (source_dest[source_ranks[i] * num_nodes + a.v] == 1) {
			complete_paths.push_back(p);
		    }
		    //printf("done adding %d %d\n", a.u, a.v);
		    
		    visited[i * num_nodes + neighbor_rank] = true;
		    done = false;
		    break;
		}
	    }
	}
    }

    gettimeofday(&t2, NULL);

    /*For the current number of paths, start expanding and add more path*/
    while(bfs->heap->num_elements != 0) 
    {
	struct path *p = hp_find_min(bfs->heap);
	//optiq_path_print_path(p);
	hp_remove_min(bfs->heap);

	struct arc a = p->arcs.back();
	int furthest_point = a.v;

	/*Expanding all paths from this point*/
	for (int i = 0; i < bfs->neighbors[furthest_point].size(); i++) 
	{
	    neighbor_rank = bfs->neighbors[furthest_point][i];

	    if (!visited[p->root_id * num_nodes + neighbor_rank]) 
	    {
		struct path *np = &bfs->paths[max_avail_path_id];
		max_avail_path_id++;

		np->arcs = p->arcs;
		np->max_load = p->max_load;
		np->root_id = p->root_id;

		struct arc na;
		na.u = a.v;
		na.v = neighbor_rank;
		np->arcs.push_back(na);

		mton_add_load_on_path(np, load, 1, num_nodes);
		mton_add_edge_path(bfs->edge_path, np, num_nodes);

		hp_insert(bfs->heap, np);

		if (source_dest[source_ranks[np->root_id] * num_nodes + na.v] == 1) {
		    complete_paths.push_back(np);
		}

		mton_update_max_load(np, load, bfs);

		visited[p->root_id * num_nodes + neighbor_rank] = true;

		//optiq_path_print_path(np);
	    }
	}
    }

    gettimeofday(&t3, NULL);

    free(bfs->edge_path);
    free(load);
    free(visited);

    /*
    long int diff = 0L;

    diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    printf("Init %ld microseconds\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Main part in %ld microseconds\n", diff);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Extend 1 %ld microseconds\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);
    printf("Extend 2 in %ld microseconds\n", diff);

    printf("Total edge time is %ld\n", mperf.add_edge_path_time);
    printf("Total load time is %ld\n", mperf.add_load_time);
    printf("Total update time is %ld\n", mperf.update_max_load_time);
    */
}
