#include "stdlib.h"
#include "iostream"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <queue>

#include "util.h"

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

/* Returns true if there is a path from source 's' to sink 't' in
 *    residual graph. Also fills parent[] to store the path */
bool bfs(int V, bool *visited, int **rGraph, int s, int t, int parent[])
{
    /* Create a visited array and mark all vertices as not visited */
    memset(visited, 0, sizeof(bool)*V);

    /* Create a queue, enqueue source vertex and mark source vertex
 *        as visited*/
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
 *        true, else false */
    return (visited[t] == true);
}

int optiq_compute_num_hops(int num_dims, int *source, int *dest)
{
    int num_hops = 0;
    for (int i = 0; i < num_dims; i++) {
	num_hops += abs(source[i] - dest[i]);
    }
    return num_hops;
}

int optiq_check_existing(int num_elements, int *list, int element)
{
    for (int i = 0; i < num_elements; i++) {
        if (list[i] == element) {
            return 1;
        }
    }

    return 0;
}


