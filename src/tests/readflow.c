#include "iostream"
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <queue>

using namespace std;

void rtrim(char *str)
{
    size_t n;
    n = strlen(str);
    while (n > 0 && isspace((unsigned char)str[n - 1])) {
	n--;
    }
    str[n] = '\0';
}
 
void ltrim(char *str)
{
    size_t n;
    n = 0;
    while (str[n] != '\0' && isspace((unsigned char)str[n])) {
	n++;
    }
    memmove(str, str + n, strlen(str) - n + 1);
}
 
void trim(char *str)
{
    rtrim(str);
    ltrim(str);
}

struct arc_t {
    int ep1;
    int ep2;
    int bw;
};

struct path {
    int num_arc;
    struct arct_t *arcs;
};

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

void getPath(int **rGraph, int num_vertices, int source, int dest) 
{
    bool *visited = (bool *)malloc(sizeof(bool) * num_vertices);
    int *parent = (int *) malloc(sizeof(int) * num_vertices);
    while (bfs(num_vertices, visited, rGraph, source, dest, parent)) {
	int prev = dest;
	while(parent[prev] != source) {
	    printf("%d <- ", prev);
	    prev = parent[prev];
	    rGraph[parent[prev], prev] = 0;
	}
	printf("%d\n", source);
    }
}

int main(int argc, char **argv)
{
    char *file_name = "flow85";
    FILE *file = fopen(file_name, "r");
 
    char buf[256];

    int num_vertices = 256;
    int **arc = (int **)malloc(sizeof(int *)*num_vertices);
    for (int i = 0; i < num_vertices; i++) {
	arc[i] = (int *)malloc(sizeof(int) * num_vertices);
	for (int j = 0; j < num_vertices; j++) {
	    arc[i][j] = 0;
	}
    }

    int source = 0, dest = 171, ep1, ep2, bw;
    while (fgets(buf, 256, file)!=NULL) {
	trim(buf);
	printf("source = %d, dest = %d\n", source, dest);

	while(strlen(buf) > 0) {
	    sscanf(buf, "%d %d %d", &ep1, &ep2, &bw);
	    printf("%d %d %d\n", ep1, ep2, bw);
	    arc[ep1][ep2] = bw;
	    fgets(buf, 256, file);
	    trim(buf);
	}

	/*Now we have the matrix, check what connects to what*/
	getPath(arc, num_vertices, source, dest);

	source++;
	dest++;
    }

    return 0;
}
