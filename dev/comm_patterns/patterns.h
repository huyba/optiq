#ifndef OPTIQ_PATTERNS_H
#define OPTIQ_PATTERNS_H

#include <vector>
#include <utility> 

void optiq_patterns_read_requests_from_file(char *filename, std::vector<std::pair<std::pair<int, int>, int > > &requests);

void optiq_patterns_convert_requests_to_sendrecvcounts(std::vector<std::pair<std::pair<int, int>, int > > &requests, int* sendcounts, int* recvcounts, int rank);

void optiq_patterns_alltoallv_from_file(char *filepath, void* &sendbuf, int* &sendcounts, int* &sdispls, void* &recvbuf, int* &recvcounts, int* &rdispls, int rank, int size);

void patterns_disjoint_contigous_multiranks_multinodes(int *sendcounts, int *recvcounts, int rank, int num_ranks, int num_ranks_per_node, int nbytes);

void disjoint_contiguous(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void subset_udistributed_ioagg(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void subset_rdistributed(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void overlap_contiguous(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio);

void disjoint_contiguous_firstk_lastk(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int k);

void optiq_pattern_half_half(char *filepath, int num_ranks, int demand);

void optiq_pattern_firstk_lastk(char *filepath, int num_ranks, int demand, int k);

#endif
