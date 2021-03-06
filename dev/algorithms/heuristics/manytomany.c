#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>

#include <vector>

#include "util.h"
#include "topology.h"
#include "manytomany.h"
#include "search_util.h"

void optiq_alg_heuristic_search_manytomany_same_dests(std::vector<struct path *> &complete_paths, int num_sources, int *source_ranks, int num_dests, int *dest_ranks, struct multibfs *bfs) 
{
    complete_paths.clear();

    bool isReverted = false;
    /*Revert the sources/dests for less computating*/
    if (num_sources > num_dests) {
	isReverted = true;

	int temp = num_sources;
	num_sources = num_dests;
	num_dests = temp;

	int *tp = source_ranks;
	source_ranks = dest_ranks;
	dest_ranks = tp;
    }

    int num_nodes = bfs->num_nodes;

    std::vector<struct path> expanding_paths;

    struct timeval t0, t1, t2, t3;

    gettimeofday(&t0, NULL);

    int *source_dest = (int *)calloc (1, sizeof(int) * num_nodes * num_nodes);
    for (int i = 0; i < num_sources; i++) {
	for (int j = 0; j < num_dests; j++) {
	    source_dest[source_ranks[i] * num_nodes + dest_ranks[j]] = 1;
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

		    add_edge_path(bfs->edge_path, p, num_nodes);
		    //printf("added edge path\n");
		    add_load_on_path(p, load, 1, num_nodes);
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

		add_load_on_path(np, load, 1, num_nodes);
		add_edge_path(bfs->edge_path, np, num_nodes);

		hp_insert(bfs->heap, np);

		if (source_dest[source_ranks[np->root_id] * num_nodes + na.v] == 1) {
		    complete_paths.push_back(np);
		}

		update_max_load(np, load, bfs);

		visited[p->root_id * num_nodes + neighbor_rank] = true;

		//optiq_path_print_path(np);
	    }
	}
    }

    gettimeofday(&t3, NULL);

    free(bfs->edge_path);
    free(load);
    free(visited);
    free(source_dest);

    hp_destroy(bfs->heap);
    free(bfs->heap);

    /*Reverted path again*/
    if (isReverted) {
	for (int i = 0; i < complete_paths.size(); i++) {
	    struct path *p = complete_paths[i];

	    /*Rever edges order*/
	    for (int j = 0; j < p->arcs.size()/2; j++) 
	    {
		struct arc a = p->arcs[j];
		p->arcs[j] = p->arcs[p->arcs.size() - j -1];
		p->arcs[p->arcs.size() - j -1] = a;
	    }

	    /*Revert endpoints order*/
	    for (int j = 0; j < p->arcs.size(); j++) 
	    {
		int temp = p->arcs[j].u;
		p->arcs[j].u = p->arcs[j].v;
		p->arcs[j].v = temp;
	    }
	}

	int *tp = source_ranks;
        source_ranks = dest_ranks;
        dest_ranks = tp;
    }

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

void optiq_alg_heuristic_search_manytomany_early_adding_load(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > all_sd, struct multibfs *bfs) 
{
    struct timeval t0, t1, t2, t3;

    gettimeofday(&t0, NULL);

    int num_nodes = bfs->num_nodes;

    std::vector<std::pair<int, std::vector<int> > > sd;
    sd.clear();

    bool isReverted = check_and_reverse (all_sd, sd, num_nodes);
    if (!isReverted) {
	sd = all_sd;
    }
    
    int num_sources = sd.size();

    int *source_dest = (int *) calloc (1, sizeof(int) * num_nodes * num_nodes);

    for (int i = 0; i < sd.size(); i++) {
        for (int j = 0; j < sd[i].second.size(); j++) {
            source_dest[sd[i].first * num_nodes + sd[i].second[j]] = 1;
        }
    }

    std::vector<struct path> expanding_paths;

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
    int neighbor_id;

    while (!done) 
    {
	done = true;
	for (int i = 0; i < sd.size(); i++) 
	{
	    int source_id = sd[i].first;

	    for (int j = 0; j < bfs->neighbors[source_id].size(); j++) 
	    {
		neighbor_id = bfs->neighbors[source_id][j];
		if (!visited[i * num_nodes + neighbor_id]) 
		{
		    struct arc a;
		    a.u = source_id;
		    a.v = neighbor_id;

		    struct path *p = &bfs->paths[max_avail_path_id];
		    max_avail_path_id++;

		    p->arcs.push_back(a);
		    p->max_load = 1;
		    p->root_id = i;
		    p->source_id = source_id;

		    add_edge_path(bfs->edge_path, p, num_nodes);
		    //printf("added edge path\n");
		    add_load_on_path(p, load, 1, num_nodes);
		    //printf("added load\n");
		    hp_insert(bfs->heap, p);
		    //printf("inserted to heap\n");
		    if (source_dest[source_id * num_nodes + a.v] == 1) {
			complete_paths.push_back(p);
		    }
		    //printf("done adding %d %d\n", a.u, a.v);
		    
		    visited[i * num_nodes + neighbor_id] = true;
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
	    neighbor_id = bfs->neighbors[furthest_point][i];

	    if (!visited[p->root_id * num_nodes + neighbor_id]) 
	    {
		struct path *np = &bfs->paths[max_avail_path_id];
		max_avail_path_id++;

		np->arcs = p->arcs;
		np->max_load = p->max_load;
		np->root_id = p->root_id;
		np->source_id = p->source_id;

		struct arc na;
		na.u = a.v;
		na.v = neighbor_id;
		np->arcs.push_back(na);

		add_load_on_path(np, load, 1, num_nodes);
		add_edge_path(bfs->edge_path, np, num_nodes);

		if (source_dest[p->source_id * num_nodes + na.v] == 1) {
		    complete_paths.push_back(np);
		} else {
		    hp_insert(bfs->heap, np);
		    visited[p->root_id * num_nodes + neighbor_id] = true;
		}

		update_max_load(np, load, bfs);
	    }
	}
    }

    gettimeofday(&t3, NULL);

    free(bfs->edge_path);
    free(load);
    free(visited);
    free(source_dest);

    hp_destroy(bfs->heap);
    free(bfs->heap);

    /*Reverted path again*/
    if (isReverted) {
	for (int i = 0; i < complete_paths.size(); i++) {
	    struct path *p = complete_paths[i];

	    /*Rever edges order*/
	    for (int j = 0; j < p->arcs.size()/2; j++) 
	    {
		struct arc a = p->arcs[j];
		p->arcs[j] = p->arcs[p->arcs.size() - j -1];
		p->arcs[p->arcs.size() - j -1] = a;
	    }

	    /*Revert endpoints order*/
	    for (int j = 0; j < p->arcs.size(); j++) 
	    {
		int temp = p->arcs[j].u;
		p->arcs[j].u = p->arcs[j].v;
		p->arcs[j].v = temp;
	    }
	}
    }

    optiq_path_assign_ids(complete_paths);

    /*
    long int diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    printf("Init time = %ld\n", diff);   

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Phase 1 time = %ld\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);
    printf("Phase 2 time = %ld\n", diff);
    */
}


void optiq_alg_heuristic_search_manytomany_late_adding_load (std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > all_sd, struct multibfs *bfs) 
{
    struct timeval t0, t1, t2, t3;

    gettimeofday(&t0, NULL);

    int num_nodes = bfs->num_nodes;

    std::vector<std::pair<int, std::vector<int> > > sd;
    sd.clear();

    bool isReverted = check_and_reverse (all_sd, sd, num_nodes);
    if (!isReverted) {
	sd = all_sd;
    }
    
    int num_sources = sd.size();

    int *source_dest = (int *) calloc (1, sizeof(int) * num_nodes * num_nodes);

    for (int i = 0; i < sd.size(); i++) {
        for (int j = 0; j < sd[i].second.size(); j++) {
            source_dest[sd[i].first * num_nodes + sd[i].second[j]] = 1;
        }
    }

    std::vector<struct path> expanding_paths;

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
    int neighbor_id;

    while (!done) 
    {
	done = true;
	for (int i = 0; i < sd.size(); i++) 
	{
	    int source_id = sd[i].first;

	    for (int j = 0; j < bfs->neighbors[source_id].size(); j++) 
	    {
		neighbor_id = bfs->neighbors[source_id][j];
		if (!visited[i * num_nodes + neighbor_id]) 
		{
		    struct arc a;
		    a.u = source_id;
		    a.v = neighbor_id;

		    struct path *p = &bfs->paths[max_avail_path_id];
		    max_avail_path_id++;

		    p->arcs.push_back(a);
		    p->max_load = 1;
		    p->root_id = i;
		    p->source_id = source_id;

		    add_edge_path(bfs->edge_path, p, num_nodes);
		    //printf("added edge path\n");
		    //printf("added load\n");
		    hp_insert(bfs->heap, p);
		    //printf("inserted to heap\n");
		    if (source_dest[source_id * num_nodes + a.v] == 1) {
			add_load_on_path(p, load, 1, num_nodes);
			complete_paths.push_back(p);
		    }
		    //printf("done adding %d %d\n", a.u, a.v);
		    
		    visited[i * num_nodes + neighbor_id] = true;
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
	    neighbor_id = bfs->neighbors[furthest_point][i];

	    if (!visited[p->root_id * num_nodes + neighbor_id]) 
	    {
		struct path *np = &bfs->paths[max_avail_path_id];
		max_avail_path_id++;

		np->arcs = p->arcs;
		np->max_load = p->max_load;
		np->root_id = p->root_id;
		np->source_id = p->source_id;

		struct arc na;
		na.u = a.v;
		na.v = neighbor_id;
		np->arcs.push_back(na);

		add_edge_path(bfs->edge_path, np, num_nodes);

		hp_insert(bfs->heap, np);

		if (source_dest[p->source_id * num_nodes + na.v] == 1) {
		    add_load_on_path(np, load, 1, num_nodes);
		    complete_paths.push_back(np);
		    update_max_load(np, load, bfs);
		}

		visited[p->root_id * num_nodes + neighbor_id] = true;

		//optiq_path_print_path(np);
	    }
	}
    }

    gettimeofday(&t3, NULL);

    free(bfs->edge_path);
    free(load);
    free(visited);
    free(source_dest);

    hp_destroy(bfs->heap);
    free(bfs->heap);

    /*Reverted path again*/
    if (isReverted) {
	for (int i = 0; i < complete_paths.size(); i++) {
	    struct path *p = complete_paths[i];

	    /*Rever edges order*/
	    for (int j = 0; j < p->arcs.size()/2; j++) 
	    {
		struct arc a = p->arcs[j];
		p->arcs[j] = p->arcs[p->arcs.size() - j -1];
		p->arcs[p->arcs.size() - j -1] = a;
	    }

	    /*Revert endpoints order*/
	    for (int j = 0; j < p->arcs.size(); j++) 
	    {
		int temp = p->arcs[j].u;
		p->arcs[j].u = p->arcs[j].v;
		p->arcs[j].v = temp;
	    }
	}
    }

    optiq_path_assign_ids(complete_paths);

    /*
    long int diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    printf("Init time = %ld\n", diff);   

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Phase 1 time = %ld\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);
    printf("Phase 2 time = %ld\n", diff);
    */
}
