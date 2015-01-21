#ifndef DATAGEN_H
#define DATAGEN_H

#include <iostream>
#include <fstream>

using namespace std;

void optiq_print_arcs_to_file(int num_dims, int *size, double cap, std::ofstream &myfile);

void optiq_print_arcs(int num_dims, int *size, double cap);
void optiq_generate_mct_data(int num_dims, int *size, int problem_size);
void optiq_generate_dataIO(int num_dims, int *size, int num_sources, int factor, int num_bridges,  int *bridgeIds);

#endif
