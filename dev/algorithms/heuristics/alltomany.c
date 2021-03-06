#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <sys/time.h>

#include <vector>

#include "util.h"
#include "topology.h"
#include "alltomany.h"

void optiq_alg_heuristic_search_alltomany(std::vector<struct path *> &complete_paths, int num_dests, int *dest_ranks, struct multibfs *bfs) 
{
    complete_paths.clear();

    int num_nodes = bfs->num_nodes;

    std::vector<struct path> expanding_paths;

    struct timeval t0, t1, t2, t3;

    gettimeofday(&t0, NULL);

    int *load = (int *)calloc(1, sizeof(int) * num_nodes * num_nodes);
    bool *visited = (bool *)calloc(1, sizeof(bool) * num_dests * num_nodes);

    bfs->paths = (struct path *) calloc (1, sizeof (struct path) * num_dests * num_nodes);
    int max_avail_path_id = 0;

    bfs->edge_path = (std::vector<struct path *> *) calloc (1, sizeof(std::vector<struct path *>) * num_nodes * num_nodes);

    /*Create a heap of paths*/
    bfs->heap = (struct heap_path *) malloc (sizeof (struct heap_path));
    hp_create(bfs->heap, num_nodes * num_dests);

    gettimeofday(&t1, NULL);

    /*Adding outgoing paths of the destination nodes first*/
    bool done = false;
    int neighbor_rank;

    while (!done) {
	done = true;
	for (int i = 0; i < num_dests; i++) 
	{
	    for (int j = 0; j < bfs->neighbors[dest_ranks[i]].size(); j++) 
	    {
		neighbor_rank = bfs->neighbors[dest_ranks[i]][j];
		if (!visited[i * num_nodes + neighbor_rank]) 
		{
		    struct arc a;
		    a.u = dest_ranks[i];
		    a.v = neighbor_rank;

		    struct path *p = &bfs->paths[max_avail_path_id];
		    max_avail_path_id++;

		    p->arcs.push_back(a);
		    p->max_load = 1;
		    p->root_id = i;
		    //p->path_id = complete_paths.size();

		    add_edge_path(bfs->edge_path, p, num_nodes);
		    //printf("added edge path\n");
		    add_load_on_path(p, load, 1, num_nodes);
		    //printf("added load\n");
		    hp_insert(bfs->heap, p);
		    //printf("inserted to heap\n");
		    complete_paths.push_back(p);
		    //printf("done adding %d %d\n", a.u, a.v);
		    
		    visited[i * num_nodes + neighbor_rank] = true;
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
    
    //hp_print(bfs->heap);

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
		//np->path_id = complete_paths.size();

		struct arc na;
		na.u = a.v;
		na.v = neighbor_rank;
		np->arcs.push_back(na);

		add_load_on_path(np, load, 1, num_nodes);
		add_edge_path(bfs->edge_path, np, num_nodes);

		hp_insert(bfs->heap, np);
		complete_paths.push_back(np);

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

    optiq_path_assign_ids(complete_paths);

    optiq_path_reverse_paths(complete_paths);


/*    long int diff = 0L;

    diff = (t1.tv_usec + 1000000 * t1.tv_sec) - (t0.tv_usec + 1000000 * t0.tv_sec);
    printf("Init %ld microseconds\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Main part in %ld microseconds\n", diff);

    diff = (t2.tv_usec + 1000000 * t2.tv_sec) - (t1.tv_usec + 1000000 * t1.tv_sec);
    printf("Extend 1 %ld microseconds\n", diff);

    diff = (t3.tv_usec + 1000000 * t3.tv_sec) - (t2.tv_usec + 1000000 * t2.tv_sec);
    printf("Extend 2 in %ld microseconds\n", diff);
*/
    /*
    printf("Total edge time is %ld\n", mperf.add_edge_path_time);
    printf("Total load time is %ld\n", mperf.add_load_time);
    printf("Total update time is %ld\n", mperf.update_max_load_time);
    */
}
