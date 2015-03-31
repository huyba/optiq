#ifndef OPTIQ_MODEL
#define OPTIQ_MODEL

#include <fstream>
#include <iostream>

#include <vector>

#include "job.h"
#include "path.h"

void optiq_model_print_graph (int num_nodes, std::vector<int> *neighbors , int capacity, std::ofstream &myfile);

void optiq_model_print_jobs (std::vector<struct job> &jobs, int num_jobs, std::ofstream &myfile);

void optiq_model_write_path_based_model_data (std::vector<struct job> jobs, char *modeldatfile, int numnodes, std::vector<int> *neighbors);

#endif
