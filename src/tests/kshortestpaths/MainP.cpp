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

#include "../multibfs/datagen.h"

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

int main(int argc, char **argv)
{
    char *filePath = argv[1];
    int num_shortest_paths = atoi(argv[2]);

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

    int job_id = 0;
    int path_id = 0;

    printf("set Nodes :=\n");
    for (int i = 0; i < num_nodes; i++) {
	printf("%d\n", i);
    }
    printf(";\n\n");

    printf("set Arcs :=\n");
    optiq_print_arcs(num_dims, size, -1);
    printf(";\n\n");

    printf("param Capacity :=\n");
    optiq_print_arcs(num_dims, size, 1680);
    printf(";\n\n");

    /*Print jobs*/
    printf("set Jobs :=\n");
    for (int i = 0; i < num_sources * num_dests; i++)
    {
	printf("%d\n", i);
    }
    printf(";\n\n");

    /*Print demand*/
    int demand = 1;
    printf("param Demand :=\n");
    for (int i = 0; i < num_sources * num_dests; i++)
    {
        printf("%d %d\n", i, demand);
    }
    printf(";\n\n");


    /*Print Paths and Path_Arcs*/
    for (int i = 0; i < num_sources; i++) 
    {
	for (int j = 0; j < num_dests; j++) 
	{
	    printf("set Paths[%d] :=\n", job_id);

	    if (source_ranks[i] != dest_ranks[j]) 
	    {
		for (int h = 0; h < num_shortest_paths; h++)
		{
		    printf("%d\n", path_id + h);
		}
	    }
	    else 
	    {
		printf("%d\n", path_id);
	    }

	    printf(";\n\n");

	    print_Yen_k_shortest_paths(filePath, num_shortest_paths, source_ranks[i], dest_ranks[j], job_id, path_id);

	    job_id++;
	}
    }
}
