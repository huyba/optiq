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

#include <fstream>
#include <stdlib.h>
#include <stdio.h>

#include "job.h"
#include "path.h"

using namespace std;


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

bool check_two_paths_disjoint(struct path *p1, struct path *p2)
{
    for (int i = 0; i < p1->arcs.size(); i++)
    {
	for (int j = 0; j < p2->arcs.size(); j++)
	{
	    if (p1->arcs[i].u == p2->arcs[j].u && p1->arcs[i].v == p2->arcs[j].v) {
		return false;
	    }
	}
    }

    return true;
}

bool check_path_disjoint(struct job *nj, struct path *pa)
{
    for (int i = 0; i < nj->paths.size(); i++)
    {
	if (check_two_paths_disjoint(nj->paths[i], pa) == false)
	{
	    return false;
	}
    }

    return true;
}

void optiq_alg_yen_distinct_shortest_paths(char *filePath, int k, struct job *nj, int &path_id)
{
    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(nj->source_id), my_graph.get_vertex(nj->dest_id));

    int i = 0;
    int trials = 0;
    while (yenAlg.has_next() && i < k && trials <= 200)
    {
	BasePath *p = yenAlg.next();
	trials++;
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

	//optiq_path_print_path (pa);

	if (check_path_disjoint(nj, pa)) 
	{
	    nj->paths.push_back(pa);
	    i++;
	    path_id++;
	} else {
	    free(pa);
	}
    }
}

void optiq_alg_yen_k_distinct_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath)
{
    int path_id = 0;

    for (int i = 0; i < jobs.size(); i++)
    {
	/*printf("job_id = %d from %d to %d\n", jobs[i].job_id, jobs[i].source_id, jobs[i].dest_id);*/
        optiq_alg_yen_distinct_shortest_paths(graphFilePath, num_paths, &jobs[i], path_id);
    }
}

void get_most_h_hops_k_shortest_paths (char *filePath, int h, int k, struct job *nj, int &path_id)
{
    Graph my_graph(filePath);

    YenTopKShortestPathsAlg yenAlg(my_graph, my_graph.get_vertex(nj->source_id), my_graph.get_vertex(nj->dest_id));

    int i = 0;
    while (yenAlg.has_next())
    {
	BasePath *p = yenAlg.next();

	if (p->Weight() >= h || i > k) 
	{
	    /*If i = 0, but even the shortest path has more than h hops, print at least one.*/
	    if (i > 0) {
		break;
	    }
	}
	i++;

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

void print_jobs_ampl(struct job *jobs, int num_jobs, std::ofstream &myfile)
{
    /*Print jobs*/
    myfile << "set Jobs :=" << endl;
    for (int i = 0; i < num_jobs; i++)
    {
	myfile << jobs[i].job_id << endl;
    }
    myfile << ";" << endl << endl;

    /*Print demand*/
    myfile << "param Demand :=" << endl;
    for (int i = 0; i < num_jobs; i++)
    {
	myfile << jobs[i].job_id << " " << jobs[i].demand << endl;
    }
    myfile << ";" << endl << endl;

    /*Print Paths and Path_Arcs*/
    for (int i = 0; i < num_jobs; i++)
    {
	myfile << "set Paths[" << jobs[i].job_id << "] :=" << endl;

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    myfile << jobs[i].paths[j]->path_id << endl;
	}

	myfile << ";" << endl << endl;

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    myfile << "set Path_Arcs[" << jobs[i].job_id << ", " << jobs[i].paths[j]->path_id << "] :=" << endl;

	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		myfile << jobs[i].paths[j]->arcs[k].u << " " << jobs[i].paths[j]->arcs[k].v << endl;
	    }

	    myfile << ";" << endl << endl;
	}
    }
}

void print_jobs_stat(struct job *jobs, int num_jobs, int num_nodes)
{
    int max_hops = 0, max_load = 0;
    int **load = (int **) calloc (1, sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++) {
	load[i] = (int *) calloc (1, sizeof(int) * num_nodes);
    }

    for (int i = 0; i < num_jobs; i++) 
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    if (max_hops < jobs[i].paths[j]->arcs.size()) 
	    {
		max_hops = jobs[i].paths[j]->arcs.size();
	    }

	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		load[jobs[i].paths[j]->arcs[k].u][jobs[i].paths[j]->arcs[k].v]++;
	    }
	}
    }

    for (int i = 0; i < num_nodes; i++)
    {
	for (int j = 0; j < num_nodes; j++)
	{
	    if (load[i][j] > 0) 
	    {
		printf("%d %d %d\n", i, j , load[i][j]);

		if (max_load < load[i][j])
		{
		    max_load = load[i][j];
		}
	    }
	}
    }

    printf("max_load = %d\n", max_load);
    printf("max_hops = %d\n", max_hops);
}

int yen_k_shortest_paths(char *graphFile, int max_hops, int num_shortest_paths, char *datFile)
{
    int num_dims = 5;
    int size[5] = {2,4,4,8,2};

    int num_nodes = 1;
    for (int i = 0; i < num_dims; i++) {
	num_nodes *= size[i];
    }

    int num_sources = num_nodes;
    int *source_ranks = (int *) malloc (sizeof(int) * num_sources);
    for (int i = 0; i < num_sources; i++) {
	source_ranks[i] = i;
    }

    int num_dests = num_nodes/64;
    int *dest_ranks = (int *) malloc (sizeof(int) * num_dests);
    for (int i = 0; i < num_dests; i++) {
	dest_ranks[i] = 32 + i * 64;
    }

    std::ofstream myfile;
    myfile.open (datFile);

    myfile << "set Nodes :=\n";
    for (int i = 0; i < num_nodes; i++) {
	myfile << i << endl;
    }
    myfile << ";\n\n";

    myfile << "set Arcs :=\n";
    //optiq_print_arcs_to_file(num_dims, size, -1, myfile);
    myfile << ";\n\n";

    myfile << "param Capacity :=\n";
    int capacity = 2048;
    //optiq_print_arcs_to_file(num_dims, size, 2048, myfile);
    myfile << ";\n\n";

    int num_jobs = num_sources * num_dests;
    struct job *jobs = (struct job *) calloc (1, sizeof(struct job) * num_jobs);

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

	    //get_Yen_k_distint_shortest_paths(filePath, num_shortest_paths, &jobs[job_id], path_id);
	    //get_Yen_k_shortest_paths(filePath, num_shortest_paths, &jobs[job_id], path_id);
	    //get_most_h_hops_k_shortest_paths(filePath, max_hops, num_shortest_paths, &jobs[job_id], path_id);

	    job_id++;
	}
    }

    print_jobs_ampl(jobs, num_jobs, myfile);

    myfile.close();

    //printf("Get most k disjoint shortest paths k = %d\n", num_shortest_paths);
    printf("k shortest paths k = %d\n", num_shortest_paths);
    //printf("At most k shortest paths with max h hops k = %d, h = %d\n", num_shortest_paths, max_hops);
    print_jobs_stat(jobs, num_jobs, num_nodes);

    return 0;
}

void optiq_alg_yen_k_shortest_paths (std::vector<struct path *> &complete_paths, std::vector<struct job> &jobs, int num_paths, char *graphFilePath)
{
    Graph my_graph(graphFilePath);

    for (int i = 0; i < jobs.size(); i++)
    {
        int source_id = jobs[i].source_id;

        int dest_id = jobs[i].dest_id;

        YenTopKShortestPathsAlg yenAlg (my_graph, my_graph.get_vertex(source_id), my_graph.get_vertex(dest_id));

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

	    jobs[i].paths.push_back(pa);
	    complete_paths.push_back(pa);
            k++;
        }
    }

    for (int i = 0; i < complete_paths.size(); i++) {
        complete_paths[i]->path_id = i;
    }
}
