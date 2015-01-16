#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "path.h"

void optiq_path_print_path(struct path *p)
{
    printf("Path: max_load = %d, #hops = %d. ", p->max_load, p->arcs.size());
    for (int j = 0; j < p->arcs.size(); j++) {
	printf("%d->", p->arcs[j].u);
    }
    printf("%d\n", p->arcs[p->arcs.size() - 1].v);
}

void optiq_path_print_stat(std::vector<struct path *> &paths, int num_nodes)
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
	struct path *p = paths[i];

	if (max_hops < p->arcs.size()) {
            max_hops = p->arcs.size();
        }

	for (int j = 0; j < p->arcs.size(); j++) {
	    load[p->arcs[j].u][p->arcs[j].v]++;
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

void optiq_path_print_paths(std::vector<struct path *> &paths)
{
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) {
        struct path *p = paths[i];

	printf("path %d num_hops = %ld\n", i, p->arcs.size());

	printf("path %d: ", i);
        for (int j = 0; j < p->arcs.size(); j++) {
            printf("%d->", p->arcs[j].u);
        }
        printf("%d\n", p->arcs.back().v);
    }
}
