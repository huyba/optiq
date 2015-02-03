#include "stdlib.h"
#include "iostream"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <queue>

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
