#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include <iostream>
#include <fstream>
#include <string.h>
#include <math.h>

#include "util.h"
#include "job.h"

void optiq_job_write_to_file (std::vector<struct job> &jobs, char *filepath)
{
    std::ofstream myfile;

    myfile.open (filepath);

    myfile << jobs[0].name << std::endl << std::endl;

    for (int i = 0; i < jobs.size(); i++)
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    myfile << "J " << jobs[i].job_id << " " << jobs[i].paths[j]->path_id << " " << jobs[i].paths[j]->flow << " " << jobs[i].source_id << " " << jobs[i].dest_id << " " << jobs[i].source_rank << " " << jobs[i].dest_rank << std::endl;
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		myfile << jobs[i].paths[j]->arcs[k].u << " " << jobs[i].paths[j]->arcs[k].v << std::endl;
	    }
	    myfile << std::endl;
	}
	myfile << std::endl;
    }

    myfile.close();
}

void optiq_job_add_path_under_load (struct job &ajob, int maxload, int** &load)
{
    for (int j = 0; j < ajob.paths.size(); j++)
    {
	bool underload = true;

	for (int k = 0; k < ajob.paths[j]->arcs.size(); k++)
	{
	    int u = ajob.paths[j]->arcs[k].u;
	    int v = ajob.paths[j]->arcs[k].v;

	    if (load[u][v] >= maxload)
	    {
		underload = false;
		break;
	    }
	}

	if (underload)
	{
	    ajob.kpaths.push_back (ajob.paths[j]);

	    for (int k = 0; k < ajob.paths[j]->arcs.size(); k++)
	    {
		int u = ajob.paths[j]->arcs[k].u;
		int v = ajob.paths[j]->arcs[k].v;

		load[u][v]++;
	    }

	    ajob.paths.erase (ajob.paths.begin() + j);
	}
    }
}

void optiq_job_read_and_select (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath, int maxload, int size, int num_ranks_per_node)
{
    optiq_job_read_from_file (jobs, paths, filepath);

    int **load = (int **) calloc (1, sizeof(int *) * size);

    for (int i = 0; i < size; i++)
    {
	load[i] = (int *) calloc (1, sizeof(int) * size);
    }

    for (int i = 0; i < jobs.size(); i++)
    {
	jobs[i].kpaths.clear();
    }

    int cload = 1;

    /* Add paths in a job that has load less than cload and update load */
    while (cload <= maxload)
    {
	for (int i = 0; i < jobs.size(); i++)
	{
	    optiq_job_add_path_under_load(jobs[i], cload, load);
	}

	cload++;
    }

    /* Check to make sure that each job has at least 1 path, if not increase the maxload*/
    for (int i = 0; i < jobs.size(); i++)
    {
	cload = maxload + 1;

	while (jobs[i].kpaths.size() == 0)
	{
	    optiq_job_add_path_under_load(jobs[i], cload, load);
	    cload++;
	}
    }

    /* Free remaining paths and copy from kpaths back to paths */
    paths.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    free (jobs[i].paths[j]);
	}

	jobs[i].paths.clear();

	jobs[i].paths = jobs[i].kpaths;

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    paths.push_back(jobs[i].paths[j]);
	}
    }
}

void optiq_job_read_from_file (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath)
{
    char fullfilename[256];
    int i = 0;

    bool exist = true;

    while (exist)
    {
	sprintf(fullfilename, "%s_%d", filepath, i);
	printf("Try to open file %s\n", fullfilename);
	exist = optiq_jobs_read_from_file(jobs, paths, fullfilename);
	i++;
    }
}

bool optiq_jobs_read_from_file (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath)
{
    FILE * fp;
    char line[256];

    if( access( filepath, F_OK ) == -1 ) {
	return false;
    }

    fp = fopen(filepath, "r");

    if (fp == NULL) {
	return false;
    }

    int job_id = 0, path_id = 0, u, v, source_id, dest_id, source_rank, dest_rank;
    int job_path_id = 0; /* This is to read the path_id in, but it is never used*/

    float flow;
    char temp[256], name[256];
    bool exist;

    fgets(name, 256, fp);

    while (fgets(line, 80, fp) != NULL)
    {
	if (line[0] == 'J')
	{
	    trim(line);
	    sscanf(line, "%s %d %d %f %d %d %d %d", temp, &job_id, &job_path_id, &flow, &source_id, &dest_id, &source_rank, &dest_rank);
	    /*printf("job_id = %d job_path_id = %d, flow = %f\n", job_id, job_path_id, flow);*/

	    struct path *p = (struct path *) calloc (1, sizeof(struct path));
	    p->job_id = job_id;
	    p->path_id = path_id;
	    p->flow = rintf(flow);
	    p->arcs.clear();

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

		/*printf("u = %d, v = %d\n", u, v);*/
	    }

	    if (p->arcs.size() > 0)
	    {
		paths.push_back(p);
		path_id++;	    /* We need distinct path_id for entire system, we route message based on path_id */

		exist = false;
		for (int i = 0; i < jobs.size(); i++)
		{
		    if (jobs[i].job_id == job_id) 
		    {
			jobs[i].paths.push_back(p);
			exist = true;
			break;
		    }
		}

		if (!exist) 
		{
		    struct job new_job;
		    new_job.paths.clear();

		    sprintf(new_job.name, "%s", name);
		    new_job.job_id = job_id;
		    new_job.source_id = source_id;
		    new_job.source_rank = source_rank;
		    new_job.dest_id = dest_id;
		    new_job.dest_rank = dest_rank;
		    new_job.paths.push_back(p);

		    jobs.push_back(new_job);
		}
	    }
	    else
	    {
		free(p);
	    }
	}
    }

    fclose(fp);
}

void optiq_job_print_jobs (std::vector<struct job> &jobs)
{
    for (int i = 0; i < jobs.size(); i++)
    {
	printf("job_id = %d source_id = %d dest_id = %d, source_rank = %d, dest_id = %d, demand = %d, #paths = %ld\n", jobs[i].job_id, jobs[i].source_id, jobs[i].dest_id, jobs[i].source_rank, jobs[i].dest_rank, jobs[i].demand, jobs[i].paths.size());

	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    printf("job_id = %d path_id = %d flow = %d\n", jobs[i].job_id, jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow);
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		printf("%d->", jobs[i].paths[j]->arcs[k].u);
	    }
	    printf("%d\n", jobs[i].paths[j]->arcs.back().v);
	}
	printf("\n");
    }
}

void optiq_job_print(std::vector<struct job> &jobs, int world_rank)
{
    printf("Rank %d has %ld jobs\n", world_rank, jobs.size());
    char strpath[1024];

    for (int i = 0; i < jobs.size(); i++)
    {
	printf("Rank %d job_id = %d source = %d dest = %d\n", world_rank, jobs[i].job_id, jobs[i].source_id, jobs[i].dest_id);
	for (int j = 0; j < jobs[i].paths.size(); j++)
	{
	    printf("Rank %d job_id = %d #paths = %ld path_id = %d flow = %d\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, jobs[i].paths[j]->flow);

	    sprintf(strpath, "%s", "");
	    for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
	    {
		sprintf(strpath, "%s%d->", strpath, jobs[i].paths[j]->arcs[k].u);
	    }
	    sprintf(strpath, "%s%d", strpath, jobs[i].paths[j]->arcs.back().v);
	    printf("Rank %d job_id = %d #paths = %ld path_id = %d %s\n", world_rank, jobs[i].job_id, jobs[i].paths.size(), jobs[i].paths[j]->path_id, strpath);
	}
    }
}

void optiq_job_map_jobs_to_source_dests (std::vector<struct job> &jobs, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    for (int i = 0; i < jobs.size(); i++)
    {
	int source_id = jobs[i].source_id;
	int dest_id = jobs[i].dest_id;
	bool found = false;

	for (int j = 0; j < source_dests.size(); j++)
	{
	    if (source_dests[j].first == source_id) 
	    {
		source_dests[j].second.push_back(dest_id);
		found = true;
		break;
	    }
	}

	if (!found)
	{
	    std::vector<int> dests;
	    dests.clear();
	    dests.push_back(dest_id);
	    std::pair<int, std::vector<int> > sd = std::make_pair(source_id, dests);
	    source_dests.push_back(sd);
	}
    }
}

void optiq_job_remove_paths_over_maxload (std::vector<struct job> &jobs, int maxload, int size, int num_ranks_per_node)
{
    int **load = (int **) calloc (1, sizeof(int *) * size);

    for (int i = 0; i < size; i++) 
    {
	load[i] = (int *) calloc (1, sizeof(int) * size);
    }

    bool finished = false;

    int iters = 0;

    while (!finished)
    {
	finished = true;

	for (int i = 0; i < jobs.size(); i++)
	{
	    /*printf("job_id = %d, pathsize = %d\n", jobs[i].job_id, jobs[i].paths.size());*/
	    bool kept = false;

	    /* Any path at index less than iters is kept */
	    while (jobs[i].paths.size() > iters && !kept)
	    {
		struct path *p = jobs[i].paths[iters];

		bool underload = true;

		for (int j = 0; j < p->arcs.size(); j++)
		{
		    int u = p->arcs[j].u / num_ranks_per_node;
		    int v = p->arcs[j].v / num_ranks_per_node;

		    if (load[u][v] >= maxload) 
		    {
			underload = false;
			break;
		    }
		}

		if (underload) 
		{
		    for (int j = 0; j < p->arcs.size(); j++)
		    {
			int u = p->arcs[j].u / num_ranks_per_node;
			int v = p->arcs[j].v / num_ranks_per_node;
			load[u][v]++;
		    }

		    kept = true;
		    finished = false;
		    /*printf("keep path id = %d\n", p->path_id);*/
		} 
		else 
		{
		    jobs[i].paths.erase(jobs[i].paths.begin() + iters);
		    /*printf("remove path id = %d\n", jobs[i].paths[iters]->path_id);*/
		}
	    }
	}

	iters++;
    }
}
