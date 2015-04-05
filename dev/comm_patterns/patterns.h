#ifndef OPTIQ_PATTERNS_H
#define OPTIQ_PATTERNS_H

#include <vector>
#include <utility> 

#include "job.h"

void optiq_pattern_firstm_lastn_to_jobs (std::vector<struct job> &jobs, int numranks, int demand, int m, int n);

void optiq_patterns_read_requests_from_file(char *filename, std::vector<struct job> &jobs);

void optiq_patterns_convert_requests_to_sendrecvcounts (std::vector<struct job> &jobs, int* sendcounts, int* recvcounts, int rank);

void optiq_patterns_alltoallv_from_file(char *filepath, std::vector<struct job> &jobs, void* &sendbuf, int* &sendcounts, int* &sdispls, void* &recvbuf, int* &recvcounts, int* &rdispls, int rank, int size);

void patterns_disjoint_contigous_multiranks_multinodes(int *sendcounts, int *recvcounts, int rank, int num_ranks, int num_ranks_per_node, int nbytes);

void disjoint_contiguous(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void subset_udistributed_ioagg(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void subset_rdistributed(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void overlap_contiguous(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void disjoint_contiguous_firstk_lastk(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int k);

void optiq_pattern_firstk_lastk(char *filepath, int num_ranks, int demand, int k);

void optiq_pattern_lastk_firstk(char *filepath, int num_ranks, int demand, int k);

/*
 * First m node talks to last n nodes. Non-overlapped.
 * */
void optiq_pattern_firstm_lastn(char *filepath, int numranks, int demand, int m, int n, bool random);

/*
 * A partition of numranks nodes will be divided into subgroups, each has size subgroupsize. 
 * All nodes in the group needs to send demand bytes of data to aggregator. Subset.
 * */
void optiq_pattern_subgroup_agg (char *filepath, int numranks, int subgroupsize, int demand);

/* 
 * 2 groups of m and n are overlapped by numoverlap
 * */
void optiq_pattern_overlap (char *filepath, int numranks, int demand, int m, int numoverlap, int n, bool random);

void optiq_pattern_m_to_n(char *filepath, int numranks, int demand, int m, int startm, int n, int startn, bool random);

void optiq_pattern_m_to_n_to_vectors (int m, int startm, int n, int startn, std::vector<std::pair<int, std::vector<int> > > &source_dests);

#endif
