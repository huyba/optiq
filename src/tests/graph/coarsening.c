#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string.h>

#include <mpi.h>

#include "optiq.h"

int optiq_get_supernode_id(map<int, int> *node_supernode, int node_id)
{
    map<int, int>::iterator iter;
    for (iter = (*node_supernode).begin(); iter != (*node_supernode).end(); iter++) {
	if (iter->first == node_id) {
	    return iter->second;
	}
    }

    return -1;
}

int optiq_job_mapping(vector<struct optiq_job> *old_jobs, vector<struct optiq_job> *new_jobs, map<int, int> *node_supernode)
{
    for (int i = 0; i < old_jobs->size(); i++) {
	struct optiq_job *new_job = (struct optiq_job *)core_memory_alloc(sizeof(struct optiq_job), "new_job", "optiq_job_mapping");

	new_job->id = (*old_jobs)[i].id;
	new_job->source = optiq_get_supernode_id(node_supernode, (*old_jobs)[i].source);
	new_job->dest = optiq_get_supernode_id(node_supernode, (*old_jobs)[i].dest);
	new_job->demand = (*old_jobs)[i].demand;

	(*new_jobs).push_back(*new_job);
    }

    return 0;
}

void optiq_model_write_data_to_file(vector<struct optiq_supernode> *supernodes, vector<struct optiq_arc> *superarcs, vector<struct optiq_job> *new_jobs)
{
    printf("set Nodes :=\n");
    for (int i = 0; i < (*supernodes).size(); i++) {
        printf("%d\n", (*supernodes)[i].id);
    }
    printf(";\n\n");

    printf("set Arcs :=\n");
    for (int i = 0; i < (*superarcs).size(); i++) {
	printf("%d %d\n", (*superarcs)[i].ep1, (*superarcs)[i].ep2);
    }
    printf(";\n\n");

    printf("param Capacity :=\n");
    for (int i = 0; i < (*superarcs).size(); i++) {
        printf("%d %d %d\n", (*superarcs)[i].ep1, (*superarcs)[i].ep2, (*superarcs)[i].capacity);
    }
    printf(";\n\n");

    printf("param: Jobs: Source Destination Demand :=\n");
    for (int i = 0; i < (*new_jobs).size(); i++) {
        printf("%d %d %d %d\n", (*new_jobs)[i].id, (*new_jobs)[i].source, (*new_jobs)[i].dest, (*new_jobs)[i].demand);
    }
    printf(";\n");
}

int main(int argc, char **argv)
{
    vector<struct optiq_job> jobs;

    const char *filePath = "../../../model/flow32";
    if (argc > 1) {
	filePath = argv[1];
    }
    optiq_job_read_from_file(filePath, &jobs);

    //optiq_job_print(&jobs);

    /*Building a graph of normal nodes*/
    vector<struct optiq_bgq_node *> nodes;
    int size[5] = {2, 4, 4, 4, 2};
    int **graph = optiq_graph_build_nodes_graph_bgq(size, &nodes);

    vector<struct optiq_supernode> supernodes;
    vector<struct optiq_arc> superarcs;
    map<int, int> node_supernode;   

    optiq_graph_coarsen_bgq(&supernodes, &superarcs, &node_supernode);   

    vector<struct optiq_job> new_jobs;
    optiq_job_mapping(&jobs, &new_jobs, &node_supernode);

    optiq_model_write_data_to_file(&supernodes, &superarcs, &new_jobs);  

    return 0;
}
