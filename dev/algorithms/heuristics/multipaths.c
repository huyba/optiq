#include "multipaths.h"

#include "Graph.h"
#include "YenTopKShortestPathsAlg.h"

void optiq_alg_heuristic_search_kpaths(std::vector<struct path *> &complete_paths, std::vector<std::pair<int, std::vector<int> > > source_dests, struct multibfs *bfs, int num_paths, char *graphFilePath)
{
    Graph my_graph(graphFilePath);

    for (int i = 0; i < source_dests.size(); i++)
    {
	int source_rank = source_dests[i].first;

	for (int j = 0; j < source_dests[i].second.size(); j++)
	{
	    int dest_rank = source_dests[i].second[j];

	    YenTopKShortestPathsAlg yenAlg (my_graph, my_graph.get_vertex(source_rank), my_graph.get_vertex(dest_rank));

	    int k = 0;
	    while (yenAlg.has_next() && k < num_paths)
	    {
		BasePath *p = yenAlg.next();

		struct path *pa = (struct path *) calloc (1, sizeof(struct path));

		for (int j = 0; j < p->m_vtVertexList.size() - 1; j++)
		{
		    struct arc a;

		    a.u = p->m_vtVertexList[j]->getID();
		    a.v = p->m_vtVertexList[j + 1]->getID();

		    pa->arcs.push_back(a);
		}

		complete_paths.push_back(pa);
		k++;
	    }
	}
    }

    for (int i = 0; i < complete_paths.size(); i++) {
	complete_paths[i]->path_id = i;
    }
}
