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

struct optiq_arc {
    int ep1;
    int ep2;
};

struct optiq_flow {
    int id;
    int throughput;
    int num_arcs;
    struct optiq_arc *arcs;
};

struct optiq_job {
    int id;
    int source;
    int dest;
    int demand;
    int num_flows;
    queue<struct optiq_flow *> flows;
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

void get_flows(int **rGraph, int num_vertices, struct optiq_job *job, int *flow_id) 
{
    int source = job->source;
    int dest = job->dest;

    bool *visited = (bool *)malloc(sizeof(bool) * num_vertices);
    int *parent = (int *) malloc(sizeof(int) * num_vertices);
    int u, v, throughput, num_arcs = 0;
    while (bfs(num_vertices, visited, rGraph, source, dest, parent)) {
	/*Get the number of arcs and throughput of the flow*/
	throughput = INT_MAX;
	num_arcs = 0;
        for (v = dest; v != source; v = parent[v]) {
            u = parent[v];
            throughput = min(throughput, rGraph[u][v]);
	    num_arcs++;
        }

	/*Create new flow*/
	struct optiq_flow *flow = (struct optiq_flow *)malloc(sizeof(struct optiq_flow));
	flow->throughput = throughput;
	flow->num_arcs = num_arcs;
	flow->arcs = (struct optiq_arc *)malloc(sizeof(struct optiq_arc) * num_arcs);
	flow->id = *flow_id;
	(*flow_id)++;

	/* Adding arcs for each flow and subtract used throughput */
	int ai = 0;
	for (v = dest; v != source; v = parent[v]) {
	    u = parent[v];
	    flow->arcs[ai].ep1 = u;
	    flow->arcs[ai].ep2 = v;
	    rGraph[u][v] -= throughput;
	    ai++;
	}

	/*Add the flow to the job*/
	job->flows.push(flow);
	job->num_flows++;
    }
}

int main(int argc, char **argv)
{
    char *file_name = "flow85";
    FILE *file = fopen(file_name, "r");
 
    char buf[256];

    int num_vertices = 256;
    int **rGraph = (int **)malloc(sizeof(int *)*num_vertices);
    for (int i = 0; i < num_vertices; i++) {
	rGraph[i] = (int *)malloc(sizeof(int) * num_vertices);
	for (int j = 0; j < num_vertices; j++) {
	    rGraph[i][j] = 0;
	}
    }

    int source = 0, dest = 171, ep1, ep2, bw;
    int flow_id = 0;
    int job_id = 0;
    int num_jobs = 85;
    struct optiq_job *jobs = (struct optiq_job *)malloc(sizeof(struct optiq_job) * num_jobs);

    while (fgets(buf, 256, file)!=NULL) {
	trim(buf);
	printf("source = %d, dest = %d\n", source, dest);

	while(strlen(buf) > 0) {
	    sscanf(buf, "%d %d %d", &ep1, &ep2, &bw);
	    printf("%d %d %d\n", ep1, ep2, bw);
	    rGraph[ep1][ep2] = bw;
	    fgets(buf, 256, file);
	    trim(buf);
	}

	/*Now we have the matrix, check what connects to what*/
	jobs[job_id].id = job_id;
	jobs[job_id].source = source;
	jobs[job_id].dest = dest;
	jobs[job_id].num_flows = 0;

	get_flows(rGraph, num_vertices, &jobs[job_id], &flow_id);
	job_id++;

	for (int i = 0; i < num_vertices; i++) {
	    for (int j = 0; j < num_vertices; j++) {
		rGraph[i][j] = 0;
	    }
	}

	source++;
	dest++;
    }

    printf("num_jobs = %d\n", num_jobs);

    struct optiq_flow *flow;
    for (int i = 0; i < num_jobs; i++) {
	printf("\njob_id = %d, source = %d , dest = %d, num_flows = %d\n", jobs[i].id, jobs[i].source, jobs[i].dest, jobs[i].num_flows);

	for (int j = 0; j < jobs[i].num_flows; j++) {
	    flow = jobs[i].flows.front();
	    jobs[i].flows.pop();

	    printf("flow_id = %d, throughput = %d, num_arcs = %d\n", flow->id, flow->throughput, flow->num_arcs);
	    for (int k = flow->num_arcs-1; k >= 0; k--) {
		printf("%d -> ", flow->arcs[k].ep1);
	    }
	    printf("%d\n", flow->arcs[0].ep2);
	}
    }

    return 0;
}
