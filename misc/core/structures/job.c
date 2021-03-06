#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <limits.h>
#include <string.h>

#include "../utils/util.h"
#include "job.h"

void build_look_up_next_dest_table(vector<struct optiq_job> &jobs, int rank, map<int, int> &next_dest)
{
    for (int i = 0; i < jobs.size(); i++) {
	for (int j = 0; j < jobs[i].flows.size(); j++) {
	    for (int k = 0; k < jobs[i].flows[j].arcs.size(); k++) {
		if (jobs[i].flows[j].arcs[k].ep1 == rank) {
		    next_dest.insert(make_pair(jobs[i].flows[j].id, jobs[i].flows[j].arcs[k].ep2));
		}
	    }
	}
    }
}

int get_next_dest_from_jobs(vector<struct optiq_job> *jobs, int flow_id, int current_ep)
{
    for (int i = 0; i < jobs->size(); i++) {
        for (int j = 0; j < (*jobs)[i].flows.size(); j++) {
            if ((*jobs)[i].flows[j].id == flow_id) {
                return get_next_dest_from_flow(&(*jobs)[i].flows[j], current_ep);
            }
        }
    }
    return -1;
}

void optiq_job_print(vector<struct optiq_job> *jobs)
{
    printf("Number of jobs = %ld\n", jobs->size());
    int num_flows = 0, total_arcs = 0, min_arcs = 1000, max_arcs = 0;
    
    struct optiq_flow flow;

    for (int i = 0; i < (*jobs).size(); i++) {
        printf("\njob_id = %d, source = %d , dest = %d, num_flows = %ld, demand = %d\n", (*jobs)[i].id, (*jobs)[i].source, (*jobs)[i].dest, (*jobs)[i].flows.size(), (*jobs)[i].demand);

	num_flows += (*jobs)[i].flows.size();

        for (int j = 0; j < (*jobs)[i].flows.size(); j++) {
            flow = (*jobs)[i].flows[j];

            printf("flow_id = %d, throughput = %d, num_arcs = %ld\n", flow.id, flow.throughput, flow.arcs.size());

	    total_arcs += flow.arcs.size();
	    if (min_arcs > flow.arcs.size()) {
		min_arcs = flow.arcs.size();
	    }
	    if (max_arcs < flow.arcs.size()) {
		max_arcs = flow.arcs.size();
	    }

            for (int k = 0; k < flow.arcs.size(); k ++) {
                printf("%d -> ", flow.arcs[k].ep1);
            }
            printf("%d\n", flow.arcs[flow.arcs.size()-1].ep2);
        }
    }

    printf("\nMin arcs = %d, Max arcs = %d, Avg arcs = %d\n", min_arcs, max_arcs, total_arcs/num_flows);
}

void optiq_job_read_from_file(const char *file_path, vector<struct optiq_job> *jobs)
{
    FILE *file = fopen(file_path, "r");

    char buf[256];

    int num_vertices = 256;
    int **rGraph = (int **)malloc(sizeof(int *)*num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        rGraph[i] = (int *)malloc(sizeof(int) * num_vertices);
        for (int j = 0; j < num_vertices; j++) {
            rGraph[i][j] = 0;
        }
    }

    int source = 0, dest = 0, ep1, ep2, bw, demand;
    int flow_id = 0;
    int job_id = 0;
    char c;

    /*Skip the first 6 lines*/
    for (int i = 0; i < 6; i++) {
	fgets(buf, 256, file);
	/*printf("%s\n", buf);*/
    }

    while (fgets(buf, 256, file)!=NULL) {
        trim(buf);
        /*printf("%s\n", buf);*/

        while(strlen(buf) > 0) {
	    if (buf[0] == 'J') {
		sscanf(buf, "%c %d %d %d %d",&c, &job_id, &source, &dest, &demand);
		/*printf("%c %d %d %d %d\n", c, job_id, source, dest, demand);*/
	    } else {
		sscanf(buf, "%d %d %d", &ep1, &ep2, &bw);
		/*printf("%d %d %d\n", ep1, ep2, bw);*/
		rGraph[ep1][ep2] = bw;
	    }
	    fgets(buf, 256, file);
	    trim(buf);
        }

        /*Now we have the matrix, check what connects to what*/
        struct optiq_job job;
        job.id = job_id;
        job.source = source;
        job.dest = dest;
	job.demand = demand;

        get_flows(rGraph, num_vertices, job, flow_id);
        (*jobs).push_back(job);
        job_id++;

        for (int i = 0; i < num_vertices; i++) {
            for (int j = 0; j < num_vertices; j++) {
                rGraph[i][j] = 0;
            }
        }

        source++;
        dest++;
    }

    fclose(file);
    for (int i = 0; i < num_vertices; i++) {
        free(rGraph[i]);
    }
    free(rGraph);
}

void get_flows(int **rGraph, int num_vertices, struct optiq_job &job, int &flow_id)
{
    int source = job.source;
    int dest = job.dest;

    bool *visited = (bool *)malloc(sizeof(bool) * num_vertices);
    int *parent = (int *) malloc(sizeof(int) * num_vertices);
    int u, v, throughput;

    while (bfs(num_vertices, visited, rGraph, source, dest, parent)) {
        /*Get the number of arcs and throughput of the flow*/
        throughput = INT_MAX;
        for (v = dest; v != source; v = parent[v]) {
            u = parent[v];
            throughput = min(throughput, rGraph[u][v]);
        }

        /*Create new flow*/
        struct optiq_flow flow;
        flow.throughput = throughput;
        flow.id = flow_id;
        flow_id++;

        /* Adding arcs for each flow and subtract used throughput */
        for (v = dest; v != source; v = parent[v]) {
            u = parent[v];
            struct optiq_arc arc;
            arc.ep1 = u;
            arc.ep2 = v;
            flow.arcs.insert(flow.arcs.begin(), arc);
            rGraph[u][v] -= throughput;
        }

        /*Add the flow to the job*/
        job.flows.push_back(flow);
    }
}
