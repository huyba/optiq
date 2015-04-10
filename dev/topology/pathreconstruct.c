#include <stdio.h>
#include <stdlib.h>

#include "util.h"

#include "pathreconstruct.h"

void optiq_topology_path_reconstruct_new (std::vector<std::pair<int, int> > &source_dests, std::vector<struct path *> &mpi_paths)
{
    for (int i = 0; i < source_dests.size(); i++)
    {
        int source_rank = source_dests[i].first;
	int dest_rank = source_dests[i].second;

        struct path *p = (struct path*) calloc (1, sizeof (struct path));

        optiq_topolog_reconstruct_path(source_rank, dest_rank, *p);

	mpi_paths.push_back(p);
    }
}

void optiq_topology_path_reconstruct(std::vector<std::pair<int, std::vector<int> > > &source_dests, struct topology *topo, std::vector<struct path *> &mpi_paths)
{
    int num_nodes = topo->num_nodes;
    int num_dims = topo->num_dims;
    int *size = topo->size;
    int *torus = topo->torus;
    int *order = topo->order;
    int **all_coords = topo->all_coords;

    int max_hops = num_nodes * 2;
    int **path = (int **)malloc(sizeof(int *) * max_hops);

    for (int i = 0; i < max_hops; i++) {
	path[i] = (int *)malloc(sizeof(int) * num_dims);
	for (int j = 0; j < num_dims; j++) {
	    path[i][j] = 0;
	}
    }

    int source_rank, dest_rank, num_hops;

    for (int i = 0; i < source_dests.size(); i ++)
    {
	int source_rank = source_dests[i].first;

	for (int j = 0; j < source_dests[i].second.size(); j++)
	{
	    int dest_rank = source_dests[i].second[j];

	    struct path *p = (struct path*) calloc (1, sizeof (struct path));

	    optiq_topology_reconstruct_path_bgq (num_dims, size, torus, order, all_coords[source_rank], all_coords[dest_rank], path);

	    num_hops = optiq_compute_num_hops(num_dims, all_coords[source_rank], all_coords[dest_rank]);

	    for (int h = 0; h < num_hops; h ++) 
	    {
		struct arc a;

		a.u = optiq_topology_compute_node_id(num_dims, size, path[h]);
		a.v = optiq_topology_compute_node_id(num_dims, size, path[h+1]);
		p->arcs.push_back(a);
	    }

	    printf("source_rank = %d , dest_rakn = %d, num_hops = %d\n", source_rank, dest_rank, num_hops);

	    for (int x = 0; x < max_hops; x++) {
		for (int y = 0; y < num_dims; y++) {
		    path[x][y] = 0;
		}
	    }

	    mpi_paths.push_back(p);
	}
    }

    for (int i = 0; i < mpi_paths.size(); i++) {
	mpi_paths[i]->path_id = i;
    }

    for (int i = 0; i < max_hops; i++) {
        free(path[i]);
    }

    free(path);
}
