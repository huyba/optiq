/* C++ program for implementation of Ford Fulkerson algorithm*/
#include "maxflow.h"
#include "topology.h"

using namespace std;

/* Returns true if there is a path from source 's' to sink 't' in
   residual graph. Also fills parent[] to store the path */
bool bfs(int V, bool *visited, int **rGraph, int s, int t, int parent[])
{
    /* Create a visited array and mark all vertices as not visited */
    memset(visited, 0, sizeof(bool)*V);

    /* Create a queue, enqueue source vertex and mark source vertex
       as visited*/
    queue <int> q;
    q.push(s);
    visited[s] = true;
    parent[s] = -1;

    /* Standard BFS Loop */
    while (!q.empty())
    {
	int u = q.front();
	q.pop();

	for (int v=0; v<V; v++)
	{
	    if (visited[v]==false && rGraph[u][v] > 0)
	    {
		q.push(v);
		parent[v] = u;
		visited[v] = true;
	    }
	}
    }

    /* If we reached sink in BFS starting from source, then return
       true, else false */
    return (visited[t] == true);
}

/* Returns tne maximum flow from s to t in the given graph */
int fordFulkerson(int V, int **graph, int *sources, int n, int t, multipath_t *mp)
{
    int u, v;

    /* Create a residual graph and fill the residual graph with
       given capacities in the original graph as residual capacities
       in residual graph */
    int **rGraph; /* Residual graph where rGraph[i][j] indicates 
		     /residual capacity of edge from i to j (if there
		     is an edge. If rGraph[i][j] is 0, then there is not) */
    rGraph = (int **)malloc(sizeof(int *)*V);
    for(int i = 0; i < V; i++)
	rGraph[i] = (int*)malloc(sizeof(int)*V);

    for (u = 0; u < V; u++)
	for (v = 0; v < V; v++)
	    rGraph[u][v] = graph[u][v];

    int *parent = (int*)malloc(sizeof(int)*V); /* This array is filled by BFS and to store path */

    bool *visited = (bool*)malloc(sizeof(bool)*V);

    int max_flow = 0;  /* There is no flow initially */

    /* Augment the flow while tere is path from source to sink */
    int i = 0;
    int s = sources[i];
    while (bfs(V, visited, rGraph, s, t, parent))
    {
	/* Find minimum residual capacity of the edhes along the
	   path filled by BFS. Or we can say find the maximum flow
	   through the path found. */
	int path_flow = INT_MAX;
	for (v=t; v!=s; v=parent[v])
	{
	    u = parent[v];
	    path_flow = min(path_flow, rGraph[u][v]);
	}

	/* update residual capacities of the edges and reverse edges
	   along the path */
	for (v=t; v != s; v=parent[v])
	{
	    u = parent[v];
	    rGraph[u][v] -= path_flow;
	    rGraph[v][u] += path_flow;
	}

	if(path_flow > 0)
	{
	    for (v=t; v != s; v=parent[v])
	    {
		u = parent[v];
		//if(u != s)
		//{
		    mp->paths[mp->num_paths].vertices[mp->paths[mp->num_paths].num_vertices] = u;
		    mp->paths[mp->num_paths].num_vertices++;
		//}
		//printf("%d<-", v);
	    }

	    /*Reverse the path*/
	    int total = mp->paths[mp->num_paths].num_vertices;
	    int mid = total/2;
	    for(int i = 0; i < mid; i++)
	    {
		int temp = mp->paths[mp->num_paths].vertices[i];
		mp->paths[mp->num_paths].vertices[i] = mp->paths[mp->num_paths].vertices[total-i-1];
		mp->paths[mp->num_paths].vertices[total-i-1] = temp;
	    }
	    mp->num_paths++;
	    //printf("%d num_paths=%d\n", s, mp->num_paths);
	}

	/* Add path flow to overall flow */
	max_flow += path_flow;
	i++;
	s = sources[i%n];
    }
    mp->max_flow = max_flow;

    /* Return the overall flow */
    return max_flow;
}

int compute_max_flow(int num_dims, int * size, int *sourceId, int nsources, int *destId, int ndests, multipath_t *mp)
{
    int V = 1;
    for(int i = 0; i < num_dims; i++)
	V *= size[i];

    int E = 0, temp = 1;
    for(int i = 0; i < num_dims; i++)
    {
	temp = 1;
	for(int j = 0; j < num_dims; j++)
	{
	    if(i == j)
		temp *= (size[i]-1);
	    else
		temp *= size[i];
	}
	E += temp;
    }

    for(int i = 0; i < nsources; i++)
	if(sourceId[i] < 0 || sourceId[i] >= V)
	{
	    cout << "source is not in range [1, " << V << "]" << endl;
	    exit(0);
	}

    for(int i = 0; i < ndests; i++)
        if(destId[i] < 0 || destId[i] >= V)
        {
            cout << "dest is not in range [1, " << V << "]" << endl;
            exit(0);
        }

    int fake_source = V;
    int fake_dest = V+1;

    V += 2;
    int **graph = (int **)malloc(sizeof(int*)*V);
    for(int i = 0; i < V; i++)
	graph[i] = (int *)malloc(sizeof(int)*V);

    int neighbors[10];
    int num_neighbors = 0;
    int coord[5];
    int BW = 2;
    int nid = 0;

    for(int i = 0; i < V; i++)
	for(int j = 0; j < V; j++)
	    graph[i][j] = 0;

    for(int ad = 0; ad < size[0]; ad++)
    {
	coord[0] = ad;
	for(int bd = 0; bd < size[1]; bd++)
	{
	    coord[1] = bd;
	    for(int cd = 0; cd < size[2]; cd++)
	    {
		coord[2] = cd;
		for(int dd = 0; dd < size[3]; dd++)
		{
		    coord[3] = dd;
		    for(int ed = 0; ed < size[4]; ed++)
		    {
			coord[4] = ed;
			num_neighbors = 0;
			nid = compute_nid(num_dims, coord, size);
			num_neighbors = compute_neighbors(num_dims, coord, size, neighbors);
			for(int i = 0; i < num_neighbors; i++)
			{
			    graph[nid][neighbors[i]] = BW;
			}
		    }
		}
	    }
	}
    }

    /*Adding links from fake source to real sources*/
    for(int i = 0; i < nsources; i++)
    {
	graph[fake_source][sourceId[i]] = INT_MAX;
    }
    for(int i = 0; i < ndests; i++)
    {
	graph[destId[i]][fake_dest] = INT_MAX;
    }

    int max_flow = fordFulkerson(V, graph, sourceId, nsources, fake_dest, mp);

    return max_flow;
}

void print_path(multipath_t *mp)
{
    printf("Max flow = %d\n", mp->max_flow);
    printf("There are %d paths\n", mp->num_paths);
    for(int i = 0; i < mp->num_paths; i++)
    {
	path_t path = mp->paths[i];
	printf("%d", path.vertices[0]);
	for(int j = 1; j < path.num_vertices; j++)
	{
	    printf("->%d", path.vertices[j]);
	}
	printf("\n");
    }
}
