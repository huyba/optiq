#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <fstream>

#include "util.h"
#include "path.h"

int max_path_length = 0;
int max_path_load = 0;

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

int opitq_path_compare_favor_load (struct path *p1, struct path *p2)
{
    if (p1->max_load >= max_path_load && p2->max_load >= max_path_load) {
	if (p1->max_load > p2->max_load) {
	    max_path_load = p1->max_load;
	    return 1;
	} else if (p1->max_load < p2->max_load) {
	    max_path_load = p2->max_load;
	    return -1;
	} else {
	    max_path_load = p2->max_load;
	    return 0;
	}
    } else if (p1->max_load >= max_path_load && p2->max_load < max_path_load) {
	max_path_load = p1->max_load;
	return 1;
    } else if (p1->max_load < max_path_load && p2->max_load >= max_path_load) {
	max_path_load = p2->max_load;
        return -1;
    } else {
	int max = p1->max_load > p2->max_load ? p1->max_load : p2->max_load;

	if (max_path_load < max) {
	    max_path_load = max;
	}

	return 0;
    }

    return 0;
}

/* 
 * 4 iterations of comparison: load - hop - load - hop
 * */
int optiq_path_compare (struct path *p1, struct path *p2)
{
    int ret = opitq_path_compare_favor_load(p1, p2);

    if (ret != 0) {
	return ret;
    } else {
	return optiq_path_compare_favor_hop (p1, p2);
    }
}

int optiq_path_compare_favor_hop (struct path *p1, struct path *p2)
{
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

void optiq_path_print_path_coords(struct path *p, int** coords)
{
    printf("Path: max_load = %d, #hops = %ld. ", p->max_load, p->arcs.size());

    for (int j = 0; j < p->arcs.size(); j++) {
        printf ("%d [%d %d %d %d %d]->", p->arcs[j].u, coords[p->arcs[j].u][0], coords[p->arcs[j].u][1], coords[p->arcs[j].u][2], coords[p->arcs[j].u][3], coords[p->arcs[j].u][4]);
    }

    printf("%d [%d %d %d %d %d]\n", p->arcs[p->arcs.size() - 1].v, coords[p->arcs[p->arcs.size() - 1].v][0], coords[p->arcs[p->arcs.size() - 1].v][1], coords[p->arcs[p->arcs.size() - 1].v][2], coords[p->arcs[p->arcs.size() - 1].v][3], coords[p->arcs[p->arcs.size() - 1].v][4]);
}

void optiq_path_print_path(struct path *p)
{
    printf("Path: max_load = %d, #hops = %ld. ", p->max_load, p->arcs.size());
    for (int j = 0; j < p->arcs.size(); j++) {
	printf("%d->", p->arcs[j].u);
    }
    printf("%d\n", p->arcs[p->arcs.size() - 1].v);
}

void optiq_path_print_stat(std::vector<struct path *> &paths, int num_nodes, int num_edges)
{
    int total_hops = 0;
    int max_hops = 0;
    int min_hops = 1000;
    float avg_hops = 0;
    int med_hops = 0;

    int total_loads = 0;
    int max_load = 0;
    int min_load = 1000;
    float avg_load = 0;
    int med_load = 0;
    

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
	if (min_hops > p->arcs.size()) {
            min_hops = p->arcs.size();
        }

	for (int j = 0; j < p->arcs.size(); j++) {
	    load[p->arcs[j].u][p->arcs[j].v]++;
	}

	total_hops += p->arcs.size();
    }

    avg_hops = (float)total_hops/paths.size();
    int loaded_links = 0;
    for (int i = 0; i < num_nodes; i++) 
    {
	for (int j = 0; j < num_nodes; j++) 
	{
	    if (load[i][j] != 0) 
	    {
		//printf("load on edge %d %d %d\n", i, j, load[i][j]);
		loaded_links++;

		if (max_load < load[i][j]) {
		    max_load = load[i][j];
		}

		if (min_load > load[i][j]) {
                    min_load = load[i][j];
                }

		total_loads += load[i][j];
	    }
	}
    }

    int *load_stat = (int *) calloc (1, sizeof(int) * (max_load+1));

    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
	    if (load[i][j] != 0) {
		load_stat[load[i][j]]++;
	    }
	}
    }

    load_stat[0] = num_edges;
    for (int i = 1; i <= max_load; i++) {
	load_stat[0] -= load_stat[i];
    }

    avg_load = (float)total_loads/loaded_links;

    printf("#paths = %d, total_hops = %d, total_loads = %d, #loaded_links = %d\n", paths.size(), total_hops, total_loads, loaded_links);

    printf("max_hop = %d\n", max_hops);
    printf("min_hop = %d\n", min_hops);
    printf("avg_hop = %4.2f\n", avg_hops);

    printf("max_load = %d\n", max_load);
    printf("min_load = %d\n", min_load);
    printf("avg_load = %4.2f\n", avg_load);

    for (int i = 0; i <= max_load; i++) {
	printf("num of links with load = %d is %d\n", i, load_stat[i]);
    }
    printf("\n");

    for (int i = 0; i < num_nodes; i++) {
        free(load[i]);
    }
    free(load);
    free(load_stat);
}

void optiq_path_write_paths(std::vector<struct path *> &paths, char *filepath)
{
    std::ofstream myfile;

    myfile.open (filepath);

    myfile << "#paths = " << paths.size() << std::endl << std::endl;

    for (int i = 0; i < paths.size(); i++)
    {
        struct path *p = paths[i];

        myfile << "J " << p->job_id << " " << p->path_id << " " << p->flow << std::endl;

        for (int j = 0; j < p->arcs.size(); j++) {
	    myfile << p->arcs[j].u << " " << p->arcs[j].v << std::endl;
        }
	myfile << std::endl;
    }

    myfile.close();
}

void optiq_path_print_paths(std::vector<struct path *> &paths)
{
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) 
    {
        struct path *p = paths[i];

	printf("path_id = %d num_hops = %ld, max_load = %d, flow = %d\n", i, p->arcs.size(), p->max_load, p->flow);

	printf("path_id = %d: ", i);
        for (int j = 0; j < p->arcs.size(); j++) {
            printf("%d->", p->arcs[j].u);
        }
        printf("%d\n", p->arcs.back().v);
    }
}

void optiq_path_print_paths_coords(std::vector<struct path *> &paths, int** coords)
{
    printf("#paths = %ld\n", paths.size());

    for (int i = 0; i < paths.size(); i++) {
        struct path *p = paths[i];

        printf("path_id = %d num_hops = %ld, max_load = %d, flow = %d\n", i, p->arcs.size(), p->max_load, p->flow);

        printf("path_id = %d: ", i);

        for (int j = 0; j < p->arcs.size(); j++) {
            printf("%d [%d %d %d %d %d]->", p->arcs[j].u, coords[p->arcs[j].u][0], coords[p->arcs[j].u][1], coords[p->arcs[j].u][2], coords[p->arcs[j].u][3], coords[p->arcs[j].u][4]);
        }

        printf("%d [%d %d %d %d %d]\n", p->arcs.back().v, coords[p->arcs.back().v][0], coords[p->arcs.back().v][1], coords[p->arcs.back().v][2], coords[p->arcs.back().v][3], coords[p->arcs.back().v][4]);
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

    int job_id = 0, path_id = 0, u, v;
    float flow;
    char temp[256];

    while (fgets(line, 80, fp) != NULL)
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
