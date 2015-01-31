#ifndef OPTIQ_UTIL
#define OPTIQ_UTIL

void trim(char *str);
void rtrim(char *str);
void ltrim(char *str);

int optiq_compute_num_hops(int num_dims, int *source, int *dest);

int optiq_check_existing(int num_elements, int *list, int element);

#endif
