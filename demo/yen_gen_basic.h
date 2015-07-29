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

/* Maximum number of paths for a test. This can be set to reduce the number of paths if there are too many node. Usually can be set to 50*1024 paths */
extern int maxpathspertest;

/* maxtest id and min test id. Can be set to limit number of paths*/
extern int maxtestid, mintestid;

/*search paths between sources and destinations in jobs and write to files*/
void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths);

/* Read jobs from CESM file format */
void optiq_job_read_jobs_from_cesm (std::vector<struct job> &jobs, int datasize, char *cesmFilePath);

/*Generate paths for CESM*/
void gen_paths_cesm (struct optiq_topology *topo, int datasize, char *graphFilePath, int numpaths, char *cesmFilePath, bool gather);

/* Generate paths with random message sizes from minsize to maxsize*/
void gen_paths_with_rand_msg(struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize);

/* Generate paths with increase distance and ratio for 2k and 4k partitions */
void gen_distance_increase_2k4k(struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

/* Generate paths for 91 cases for 3 patterns */
void gen_91_cases (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

/* Generate paths for 1/16 part*/
void gen_1_16_to_1_2 (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing);

void gen_multiranks (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing);

void gen_multiranks2 (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing, int start, int s1, int num_node1, int s2, int num_node2);

#endif
