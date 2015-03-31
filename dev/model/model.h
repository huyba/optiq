#ifndef OPTIQ_MODEL
#define OPTIQ_MODEL

#include <fstream>
#include <iostream>

#include <vector>

#include "job.h"
#include "path.h"

void optiq_model_print_graph (int num_nodes, std::vector<int> *neighbors , int capacity, std::ofstream &myfile);

void optiq_model_print_jobs (std::vector<struct job> &jobs, std::ofstream &myfile);

void optiq_model_write_path_based_model_data (char *modeldatfile, std::vector<struct job> &jobs, int numnodes, std::vector<int> *neighbors);

bool optiq_model_read_flow_value_from_file (char *filePath, std::vector<struct job> &jobs);

#endif
