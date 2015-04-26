#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "util.h"
#include "model.h"

void optiq_model_print_graph (int num_nodes, std::vector<int> *neighbors , int capacity, std::ofstream &myfile)
{
    myfile << "set Nodes :=\n";
    for (int i = 0; i < num_nodes; i++) {
        myfile << i << std::endl;
    }
    myfile << ";\n\n";

    myfile << "set Arcs :=\n";
    for (int i = 0; i < num_nodes; i++) 
    {
    for (int j = 0; j < neighbors[i].size(); j++)
    {
        myfile << i << " " << neighbors[i][j] << std::endl;
    }
    }
    myfile << ";\n\n";

    myfile << "param Capacity :=\n";
    for (int i = 0; i < num_nodes; i++) 
    {
        for (int j = 0; j < neighbors[i].size(); j++)
        {
            myfile << i << " " << neighbors[i][j] << " " << capacity << std::endl;
        }
    }
    myfile << ";\n\n";
}

void optiq_model_print_jobs (std::vector<struct job> &jobs, std::ofstream &myfile)
{
    /*Print jobs*/
    myfile << "set Jobs :=" << std::endl;
    for (int i = 0; i < jobs.size(); i++)
    {
        myfile << jobs[i].job_id << std::endl;
    }
    myfile << ";" << std::endl << std::endl;

    /*Print demand*/
    myfile << "param Demand :=" << std::endl;
    for (int i = 0; i < jobs.size(); i++)
    {
        myfile << jobs[i].job_id << " " << jobs[i].demand << std::endl;
    }
    myfile << ";" << std::endl << std::endl;

    /*Print Paths and Path_Arcs*/
    for (int i = 0; i < jobs.size(); i++)
    {
        myfile << "set Paths[" << jobs[i].job_id << "] :=" << std::endl;

        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            myfile << jobs[i].paths[j]->path_id << std::endl;
        }

        myfile << ";" << std::endl <<std::endl;

        for (int j = 0; j < jobs[i].paths.size(); j++)
        {
            myfile << "set Path_Arcs[" << jobs[i].job_id << ", " << jobs[i].paths[j]->path_id << "] :=" << std::endl;

            for (int k = 0; k < jobs[i].paths[j]->arcs.size(); k++)
            {
                myfile << jobs[i].paths[j]->arcs[k].u << " " << jobs[i].paths[j]->arcs[k].v << std::endl;
            }

            myfile << ";" << std::endl << std::endl;
        }
    }
}

void optiq_model_write_path_based_model_data (char *modeldatfile, std::vector<struct job> &jobs, int numnodes, std::vector<int> *neighbors)
{   
    std::ofstream myfile;

    myfile.open (modeldatfile);

    /*Write graph info*/
    int capacity = 2048;
    optiq_model_print_graph (numnodes, neighbors, capacity, myfile);

    /* Write jobs data*/
    optiq_model_print_jobs (jobs, myfile);

    myfile.close();
}


bool optiq_model_read_flow_value_from_file (char *filePath, std::vector<struct job> &jobs)
{
    FILE * fp;
    char line[256];

    if( access( filePath, F_OK ) == -1 ) {
        return false;
    }

    fp = fopen(filePath, "r");

    if (fp == NULL) {
    return false;
    }

    for (int i = 0; i < 6; i++)
    {
        fgets(line, 80, fp);
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

        for (int i = 0; i < jobs.size(); i++)
        {
	if (jobs[i].job_id == job_id) 
	{
	    for (int j = 0; j < jobs[i].paths.size(); j++)
	    {
	    if (jobs[i].paths[j]->path_id == path_id) {
	        jobs[i].paths[j]->flow = flow;
	    }
	    }
	}
        }

        }
    }

    fclose(fp);
}

