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

void optiq_job_read_jobs_from_ca2xRearr (std::vector<struct job> &jobs, int datasize, char *cesmFilePath);

void gen_paths_cesm (struct optiq_topology *topo, int datasize, char *graphFilePath, int numpaths, char *cesmFilePath);

void gen_distance_increase_2k4k(struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

void gen_91_cases (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

#endif
