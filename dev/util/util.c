#include "stdlib.h"
#include "iostream"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <queue>
#include <vector>

#include "util.h"

using namespace std;

void rtrim (char *str)
{
    size_t n;
    n = strlen (str);

    while (n > 0 && isspace((unsigned char)str[n - 1])) 
    {
        n--;
    }
    str[n] = '\0';
}

void ltrim (char *str)
{
    size_t n;
    n = 0;

    while (str[n] != '\0' && isspace((unsigned char)str[n])) 
    {
        n++;
    }

    memmove (str, str + n, strlen(str) - n + 1);
}

void trim (char *str)
{
    rtrim(str);
    ltrim(str);
}

int optiq_compute_num_hops (int num_dims, int *source, int *dest)
{
    int num_hops = 0;

    for (int i = 0; i < num_dims; i++) 
    {
	num_hops += abs(source[i] - dest[i]);
    }

    return num_hops;
}

int optiq_compute_num_hops_with_torus (int num_dims, int *source, int *dest, int *torus, int *size)
{
    int num_hops = 0;

    for (int i = 0; i < num_dims; i++)
    {
        int distance = abs(source[i] - dest[i]);

	if (torus[i] == 1 && distance > size[i] / 2) {
	    distance = size[i] - distance;
	}

	num_hops += distance;
    }

    return num_hops;
}

int optiq_check_existing (int num_elements, int *list, int element)
{
    for (int i = 0; i < num_elements; i++) 
    {
        if (list[i] == element) 
	{
            return 1;
        }
    }

    return 0;
}

void optiq_util_print_source_dests(std::vector<std::pair<int, std::vector<int> > > source_dest_ids)
{
    for (int i = 0; i < source_dest_ids.size(); i++) {
        for (int j = 0; j < source_dest_ids[i].second.size(); j++) {
            printf("Source = %d, dest = %d\n", source_dest_ids[i].first, source_dest_ids[i].second[j]);
        }
    }
}

void optiq_util_randomize_source_dests (std::vector<std::pair<int, int> > &source_dests)
{
    srand (time(NULL));
    int temp = 0, k = 0;

    for (int i = 0; i < source_dests.size(); i++) 
    {
        k = rand() % source_dests.size();
        temp = source_dests[i].second;
        source_dests[i].second = source_dests[k].second;
        source_dests[k].second = temp;
    }
}
