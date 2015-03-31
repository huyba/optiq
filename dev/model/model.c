#include <stdio.h>
#include <stdlib.h>

#include "model.h"

void optiq_model_print_graph (int num_nodes, std::vector<int> *neighbors , int capacity, std::ofstream &myfile)
{
    myfile << "set Nodes :=\n";
    for (int i = 0; i < num_nodes; i++) {
        myfile << i << endl;
    }
    myfile << ";\n\n";

    myfile << "set Arcs :=\n";
    for (int i = 0; i < num_nodes; i++) 
    {
	for (int j = 0; j < neighbors[i].size(); j++)
	{
	    myfile << i << " " << neighbors[i][j] << endl;
	}
    }
    myfile << ";\n\n";

    myfile << "param Capacity :=\n";
    int capacity = 2048;
    for (int i = 0; i < num_nodes; i++) 
    {
        for (int j = 0; j < neighbors[i].size(); j++)
        {
            myfile << i << " " << neighbors[i][j] << " " << capacity << endl;
        }
    }
    myfile << ";\n\n";
}

void optiq_model_print_jobs (std::vector<struct job> &jobs, int num_jobs, std::ofstream &myfile)
{
    /*Print jobs*/
    myfile << "set Jobs :=" << endl;
    for (int i = 0; i < jobs.size(); i++)
    {
        myfile << jobs[i].job_id << endl;
    }
    myfile << ";" << endl << endl;

    /*Print demand*/
    myfile << "param Demand :=" << endl;
    for (int i = 0; i < jobs.size(); i++)
    {
        myfile << jobs[i].job_id << " " << jobs[i].demand << endl;
    }
    myfile << ";" << endl << endl;

    /*Print Paths and Path_Arcs*/
    for (int i = 0; i < jobs.size(); i++)
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

void optiq_model_write_path_based_model_data (std::vector<struct job> &jobs, char *modeldatfile, int numnodes, std::vector<int> *neighbors)
{   
    ofstream myfile;

    myfile.open (jobFile);

    /*Write graph info*/
    optiq_model_print_graph (num_nodes, neighbors, capacity, myfile);

    /* Write jobs data*/
    optiq_model_print_jobs_ampl (jobs, myfile);

    myfile.close();
}
