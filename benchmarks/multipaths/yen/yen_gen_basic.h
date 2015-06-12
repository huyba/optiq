#ifndef OPTIQ_YEN_GEN_BASIC
#define OPTIQ_YEN_GEN_BASIC

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <vector>

#include <mpi.h>

#include "yen.h"
#include "topology.h"
#include "job.h"
#include "util.h"
#include "patterns.h"

extern int maxpathspertest;

extern int maxtestid, mintestid;

void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths);

void optiq_job_read_jobs_from_cesm (std::vector<struct job> &jobs, int datasize, char *cesmFilePath);

void gen_paths_cesm (struct optiq_topology *topo, int datasize, char *graphFilePath, int numpaths, char *cesmFilePath, bool gather);

void gen_paths_with_rand_msg(struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize);

void gen_distance_increase_2k4k(struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

void gen_91_cases (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

void gen_1_16_to_1_2 (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing);

void gen_multiranks (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing);
#endif
