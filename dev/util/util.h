#ifndef OPTIQ_UTIL
#define OPTIQ_UTIL

#include <vector>

void trim(char *str);
void rtrim(char *str);
void ltrim(char *str);

/* Compute number of hops between 2 nodes */
int optiq_compute_num_hops(int num_dims, int *source, int *dest);

/* Compute number of noes with torus considered */
int optiq_compute_num_hops_with_torus (int num_dims, int *source, int *dest, int *torus, int *size);

/* Check if an element existed in the list */
int optiq_check_existing(int num_elements, int *list, int element);

/* Print soure dest of a pair*/
void optiq_util_print_source_dests(std::vector<std::pair<int, std::vector<int> > > source_dest_ids);

/* Randomize the source/dest pairing */
void optiq_util_randomize_source_dests (std::vector<std::pair<int, int> > &source_dests);

/* Print out the mem info using special routine in BGQ */
void optiq_util_print_mem_info(int rank);

#endif
