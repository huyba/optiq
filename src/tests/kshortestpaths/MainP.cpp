/************************************************************************/
/* $Id: MainP.cpp 65 2010-09-08 06:48:36Z yan.qi.asu $                                                                 */
/************************************************************************/

#include <limits>
#include <set>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "GraphElements.h"
#include "Graph.h"
#include "DijkstraShortestPathAlg.h"
#include "YenTopKShortestPathsAlg.h"

#include <stdlib.h>
#include <stdio.h>

#include "../multibfs/datagen.h"
#include "../multibfs/path.h"
#include "../multibfs/job.h"

using namespace std;


void testDijkstraGraph(char *filePath, int source, int dest)
{
    Graph* my_graph_pt = new Graph(filePath);
    DijkstraShortestPathAlg shortest_path_alg(my_graph_pt);
    BasePath* result =
	shortest_path_alg.get_shortest_path(
		my_graph_pt->get_vertex(source), my_graph_pt->get_vertex(dest));
    result->PrintOut(cout);
}

void testYenAlg(char *filePath, int k, int source, int dest)
{
    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(source),
	    my_graph.get_vertex(dest));

    int i=0;
    while(yenAlg.has_next() && i < k)
    {
	++i;
	yenAlg.next()->PrintOut(cout);
    }
}

void print_Yen_k_shortest_paths(char *filePath, int k, int source, int dest, int job_id, int &path_id)
{

    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(source), my_graph.get_vertex(dest));

    int i=0;
    while(yenAlg.has_next() && i < k)
    {
        ++i;
	BasePath *p = yenAlg.next();

	printf("set Path_Arcs[%d, %d]\n", job_id, path_id);

	for (int j = 0; j < p->m_vtVertexList.size() - 1; j++)
	{
	    printf("%d %d\n", p->m_vtVertexList[j]->getID(), p->m_vtVertexList[j + 1]->getID());
	}

	printf(";\n\n");

	path_id++;
    }
}

void get_Yen_k_shortest_paths(char *filePath, int k, struct job *nj, int &path_id)
{

    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(nj->source_id), my_graph.get_vertex(nj->dest_id));

    int i=0;
    while(yenAlg.has_next() && i < k)
    {
        ++i;
        BasePath *p = yenAlg.next();

	struct path *pa = (struct path *) calloc (1, sizeof(struct path));
	
	pa->job_id = nj->job_id;
	pa->path_id = path_id;

        for (int j = 0; j < p->m_vtVertexList.size() - 1; j++)
        {
	    struct arc a;

	    a.u = p->m_vtVertexList[j]->getID();
	    a.v = p->m_vtVertexList[j + 1]->getID();

	    pa->arcs.push_back(a);
        }

	nj->paths.push_back(pa);

        path_id++;
    }
}

void get_most_h_hops_paths (char *filePath, int h, struct job *nj, int &path_id)
{
    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(nj->source_id), my_graph.get_vertex(nj->dest_id));

    while (yenAlg.has_next())
    {
        BasePath *p = yenAlg.next();

	if (p->Weight() > h) {
	    break;
	}

        struct path *pa = (struct path *) calloc (1, sizeof(struct path));

        pa->job_id = nj->job_id;
        pa->path_id = path_id;

        for (int j = 0; j < p->m_vtVertexList.size() - 1; j++)
        {
            struct arc a;

            a.u = p->m_vtVertexList[j]->getID();
            a.v = p->m_vtVertexList[j + 1]->getID();

            pa->arcs.push_back(a);
        }

        nj->paths.push_back(pa);

        path_id++;
    }
}

void print_jobs_ampl(struct job *jobs, int num_jobs)
{
    /*Print jobs*/
    printf("set Jobs :=\n");
    for (int i = 0; i < num_jobs; i++)
    {
        printf("%d\n", jobs[i].job_id);
    }
    printf(";\n\n");

    /*Print demand*/
    printf("param Demand :=\n");
    for (int i = 0; i < num_jobs; i++)
    {
        printf("%d %d\n", jobs[i].job_id, jobs[i].demand);
    }
    printf(";\n\n");

    /*Print Paths and Path_Arcs*/
    for (int i = 0; i < num_jobs; i++)
    {
        printf("set Paths[%d] :=\n", jobs[i].job_id);

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
            printf("%d\n", jobs[i].paths[j]->path_id);
        }

        printf(";\n\n");

	for (int j = 0; j < jobs[i].paths.size(); j++)
        {
	    printf("set Path_Arcs[%d, %d]\n", jobs[i].job_id, jobs[i].paths[j]->path_id);

	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		printf("%d %d\n", jobs[i].paths[j]->arcs[k].u, jobs[i].paths[j]->arcs[k].v);
	    }

	    printf(";\n\n");
	}
    }
}


int main(int argc, char **argv)
{
    int num_dims = 5;
    int size[5] = {2,4,4,4,2};

    int num_nodes = 256;
    int num_sources = 256;
    int num_dests = 4;

    int *source_ranks = (int *) malloc (sizeof(int) * num_sources);
    for (int i = 0; i < num_sources; i++) {
	source_ranks[i] = i;
    }
    int dest_ranks[4] = {32, 96, 160, 224};

    printf("set Nodes :=\n");
    for (int i = 0; i < num_nodes; i++) {
	printf("%d\n", i);
    }
    printf(";\n\n");

    printf("set Arcs :=\n");
    optiq_print_arcs(num_dims, size, -1);
    printf(";\n\n");

    printf("param Capacity :=\n");
    int capacity = 2048;
    optiq_print_arcs(num_dims, size, 2048);
    printf(";\n\n");

    int num_jobs = num_sources * num_dests;
    struct job *jobs = (struct job *) calloc (1, sizeof(struct job) * num_jobs);

    char *filePath = argv[1];
    int num_shortest_paths = atoi(argv[2]);
    int max_hops = atoi(argv[2]);

    /*Get k shortest paths between each pair of source and destination*/
    int job_id = 0;
    int path_id = 0;
    int demand = 2048;

    for (int i = 0; i < num_sources; i++) 
    {
	for (int j = 0; j < num_dests; j++) 
	{
	    jobs[job_id].job_id = job_id;
	    jobs[job_id].source_id = source_ranks[i];
	    jobs[job_id].dest_id = dest_ranks[j];
	    jobs[job_id].demand = demand;

	    //get_Yen_k_shortest_paths(filePath, num_shortest_paths, &jobs[job_id], path_id);
	    get_most_h_hops_paths(filePath, max_hops, &jobs[job_id], path_id);

	    job_id++;
	}
    }

    print_jobs_ampl(jobs, num_jobs);
}
