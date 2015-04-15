#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include "topology.h"
#include "multibfs.h"

struct multibfs_perf mperf;

struct multibfs *bfs = NULL;

void optiq_multibfs_init()
{
    if (bfs == NULL)
    {
	bfs = (struct multibfs *) calloc (1, sizeof(struct multibfs));

	bfs->num_dims = 5;
	optiq_topology_get_size_bgq(bfs->size);
	bfs->num_nodes = 1;
	bfs->diameter = 0;

	for (int i = 0; i < bfs->num_dims; i++)
	{
	    bfs->num_nodes *= bfs->size[i];
	    bfs->diameter += bfs->size[i];
	}

	bfs->neighbors = optiq_topology_get_all_nodes_neighbors(bfs->num_dims, bfs->size);
    }
}

struct multibfs *optiq_multibfs_get()
{
    if (bfs == NULL) {
	optiq_multibfs_init();
    }

    return bfs;
}

void add_load_on_path(struct path *np, int *load, int adding_load, int num_nodes)
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

void update_max_load(struct path *np, int *load, struct multibfs *bfs)
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

void add_edge_path(std::vector<struct path*> *edge_path, struct path *p, int num_nodes)
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

void optiq_multibfs_finalize()
{
    free(bfs);
}

void optiq_multibfs_print()
{
    printf("BFS diameter %d\n", bfs->diameter);
}
