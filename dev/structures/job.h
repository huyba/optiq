/*
 * Contains header for job.
 * */

#ifndef OPTIQ_JOB_H
#define OPTIQ_JOB_H

#include <vector>
#include <opi.h>
#include "path.h"

struct optiq_memregion;

/* job struture */
struct job {
    char name[256];
    int job_id;
    int source_id;
    int dest_id;
    int source_rank;
    int dest_rank;
    int demand;

    std::vector<struct path *> paths;
    std::vector<struct path *> kpaths;
};

/* Add one more path for a pair of source and destination between */
bool optiq_job_add_one_path_under_load (struct job &ajob, int maxload, int** &load);

/* Read a job from file */
void optiq_job_read_from_file (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath);

/* Read rank and demand from file - for CESM */
bool optiq_jobs_read_rank_demand(char *filepath, std::vector<struct job> &jobs, int ion, int num_ranks_per_node, int &job_id, int element_size);

/* Read and select path based on heurisic 1*/
void optiq_job_read_and_select (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath, int maxload, int size, int num_ranks_per_node);

/* Read jobs from file*/
bool optiq_jobs_read_from_file (std::vector<struct job> &jobs, std::vector<struct path*> &paths, char *filepath);

/* Write jobs to file */
void optiq_job_write_to_file (std::vector<struct job> &jobs, char *filepath);

/* Print jobs of certain rank*/
void optiq_job_print(std::vector<struct job> &jobs, int rank);

/* Print jobs */
void optiq_job_print_jobs (std::vector<struct job> &jobs);

/* Map jobs to pairs of sources/destinations */
void optiq_job_map_jobs_to_source_dests (std::vector<struct job> &jobs, std::vector<std::pair<int, std::vector<int> > > &source_dests);

/* Remove paths that are over maxload */
void optiq_job_remove_paths_over_maxload (std::vector<struct job> &jobs, int maxload, int size, int num_ranks_per_node);

/* Write jobs into model data file format */
void optiq_job_write_jobs_model_format (char *filekpath, int maxload, int size, int num_ranks_per_node, std::vector<int> *neighbors, int capacity, char *modeldat, int max_num_paths);

/* Convert node id of a jobs to ranks when multiple ranks are used */
void optiq_jobs_convert_ids_to_ranks (std::vector<struct job> &jobs, std::vector<struct path *> &path_ids, int num_ranks_per_node);

/* Read and calulate statistics of a jobs */
void optiq_opi_jobs_stat(std::vector<struct job> &jobs);

/* Comparable function to compare 2 jobs based on their demands */
struct JobDemandComp
{
   bool operator()(const job& s1, const job& s2)
   {
       return s1.demand < s2.demand;
   }
};

/* Assign flow value based on heuristic 2 algorithm */
void optiq_job_assign_flow_value (std::vector<struct job> &jobs, int num_nodes, int unit, int demand);

#endif
