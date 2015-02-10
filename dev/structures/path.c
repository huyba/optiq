#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "util.h"
#include "path.h"

int max_path_length = 0;

int optiq_path_compare_by_max_load(struct path *p1, struct path *p2)
{
    if (p1->max_load > p2->max_load) {
	return 1;
    } else if (p1->max_load < p2->max_load) {
	return -1;
    } else {
	if (p1->arcs.size() > p2->arcs.size()) {
	    return 1;
	} else if (p1->arcs.size() < p2->arcs.size()) {
	    return -1;
	} else {
	    return 0;
	}
    }
}

int optiq_path_compare(struct path *p1, struct path *p2)
{
    if (max_path_length == 0) {
	return optiq_path_compare_by_max_load(p1, p2);
    }

    int radius = max_path_length;

    if (p1->arcs.size() >= radius && p2->arcs.size() >= radius) {
	if (p1->arcs.size() > p2->arcs.size()) {
            return 1;
        } else if (p1->arcs.size() < p2->arcs.size()) {
            return -1;
        }
    }
    else if (p1->arcs.size() >= radius && p2->arcs.size() < radius) {
	return 1;
    }
    else if (p1->arcs.size() < radius && p2->arcs.size() >= radius) {
	return -1;
    }
    
    if (p1->max_load > p2->max_load) {
        return 1;
    } else if (p1->max_load < p2->max_load) {
        return -1;
    } else {
        if (p1->arcs.size() > p2->arcs.size()) {
            return 1;
        } else if (p1->arcs.size() < p2->arcs.size()) {
            return -1;
        } else {
            return 0;
        }
    }
}

void optiq_path_print_path(struct path *p)
{
    printf("Path: max_load = %d, #hops = %ld. ", p->max_load, p->arcs.size());
    for (int j = 0; j < p->arcs.size(); j++) {
	printf("%d->", p->arcs[j].u);
    }
    printf("%d\n", p->arcs[p->arcs.size() - 1].v);
}

void optiq_path_print_stat(std::vector<struct path *> &paths, int num_nodes)
{
    int max_hops = 0;
    int max_load = 0;

    int **load = (int **)malloc(sizeof(int *) * num_nodes);
    for (int i = 0; i < num_nodes; i++)
    {
	load[i]  = (int *)malloc(sizeof(int) * num_nodes);
	memset(load[i], 0, num_nodes * sizeof(int));
    }

    for (int i = 0; i < paths.size(); i++) 
    {
	struct path *p = paths[i];

	if (max_hops < p->arcs.size()) {
            max_hops = p->arcs.size();
        }

	for (int j = 0; j < p->arcs.size(); j++) {
	    load[p->arcs[j].u][p->arcs[j].v]++;
	}
    }

    for (int i = 0; i < num_nodes; i++) {
	for (int j = 0; j < num_nodes; j++) {
	    if (load[i][j] != 0) {
		//printf("load on edge %d %d %d\n", i, j, load[i][j]);

		if (max_load < load[i][j]) {
		    max_load = load[i][j];
		}
	    }
	}
    }

    printf("max_hop = %d\n", max_hops);
    printf("max_load = %d\n", max_load);
}

void optiq_path_print_paths(std::vector<struct path *> &paths)
{
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) {
        struct path *p = paths[i];

	printf("path %d num_hops = %ld, max_load = %d, flow = %d\n", i, p->arcs.size(), p->max_load, p->flow);

	printf("path %d: ", i);
        for (int j = 0; j < p->arcs.size(); j++) {
            printf("%d->", p->arcs[j].u);
        }
        printf("%d\n", p->arcs.back().v);
    }
}


void optiq_path_read_from_file(char *filePath, std::vector<struct path *> &complete_paths)
{
    FILE * fp;
    char line[256];

    fp = fopen(filePath, "r");
    if (fp == NULL) {
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < 6; i++)
    {
        fgets(line, 80, fp);
    }

    int job_id = 0, path_id = 0, u, v;
    float flow;
    char temp[256];

    while(fgets(line, 80, fp) != NULL)
    {
        //printf("%s", line);

        if (line[0] == 'J')
        {
            trim(line);
            sscanf(line, "%s %d %d %f", temp, &job_id, &path_id, &flow);
            //printf("job_id = %d path_id = %d, flow = %f\n", job_id, path_id, flow);

            struct path *p = (struct path *) calloc (1, sizeof(struct path));
            p->job_id = job_id;
            p->path_id = path_id;
	    p->flow = rintf(flow);

            while(fgets(line,80,fp) != NULL)
            {
                trim(line);
                if (strcmp(line, "") == 0)
                {
                    break;
                }
                sscanf(line, "%d %d", &u, &v);

                struct arc a;
                a.u = u;
                a.v = v;

                p->arcs.push_back(a);

                //printf("u = %d, v = %d\n", u, v);
            }

            complete_paths.push_back(p);
        }
    }

    fclose(fp);
}

void optiq_path_assign_ids(std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
	complete_paths[i]->path_id = i;
    }
}

void optiq_path_reverse_paths (std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
	struct path *p = complete_paths[i];
	int n = p->arcs.size();

	for (int j = 0; j < n/2; j++)
	{
	    struct arc a = p->arcs[j];
	    p->arcs[j] = p->arcs[n - j - 1];
	    p->arcs[n - j - 1] = a;
	}

	for (int j = 0; j < n; j++)
	{
	    int temp = p->arcs[j].u;
	    p->arcs[j].u = p->arcs[j].v;
	    p->arcs[j].v = temp;
	}
    }
}
