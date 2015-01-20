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
    //Graph my_graph("../data/test_6_2");
    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(source),
	    my_graph.get_vertex(dest));

    int i=0;
    while(yenAlg.has_next() && i < k)
    {
	++i;
	yenAlg.next()->PrintOut(cout);
    }

    // 	System.out.println("Result # :"+i);
    // 	System.out.println("Candidate # :"+yenAlg.get_cadidate_size());
    // 	System.out.println("All generated : "+yenAlg.get_generated_path_size());

}

int main(int argc, char **argv)
{
    cout << "Welcome to the real world!" << endl;

    char *filePath = argv[1];
    int k = atoi(argv[2]);
    int source = 0;
    int dest = 64;

    testDijkstraGraph(filePath, source, dest);
    testYenAlg(filePath, k, source, dest);
}
