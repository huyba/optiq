#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <queue>
#include <vector>

#include <spi/include/kernel/memory.h>


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

void optiq_util_print_mem_info(int rank)
{
    uint64_t shared, persist, heapavail, stackavail, stack, heap, guard, mmap;

    Kernel_GetMemorySize(KERNEL_MEMSIZE_SHARED, &shared);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_PERSIST, &persist);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAPAVAIL, &heapavail);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_STACKAVAIL, &stackavail);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_STACK, &stack);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_HEAP, &heap);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_GUARD, &guard);
    Kernel_GetMemorySize(KERNEL_MEMSIZE_MMAP, &mmap);

    printf("Rank %d Allocated heap: %.2f MB, avail. heap: %.2f MB\n", rank, (double)heap/(1024*1024),(double)heapavail/(1024*1024));
    printf("Rank %d Allocated stack: %.2f MB, avail. stack: %.2f MB\n", rank, (double)stack/(1024*1024), (double)stackavail/(1024*1024));
    printf("Rank %d Memory: shared: %.2f MB, persist: %.2f MB, guard: %.2f MB, mmap: %.2f MB\n", rank, (double)shared/(1024*1024), (double)persist/(1024*1024), (double)guard/(1024*1024), (double)mmap/(1024*1024));
}
