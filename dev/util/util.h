#ifndef OPTIQ_UTIL
#define OPTIQ_UTIL

#include <vector>

void trim(char *str);
void rtrim(char *str);
void ltrim(char *str);

int optiq_compute_num_hops(int num_dims, int *source, int *dest);

int optiq_compute_num_hops_with_torus (int num_dims, int *source, int *dest, int *torus, int *size);

int optiq_check_existing(int num_elements, int *list, int element);

void optiq_util_print_source_dests(std::vector<std::pair<int, std::vector<int> > > source_dest_ids);

#endif
