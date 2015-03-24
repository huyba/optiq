#include <mpi.h>
#include <stdlib.h>
#include "patterns.h"

int main(int argc, char **argv)
{
    int rank = 0;
    int size = atoi(argv[1]);

    char *base = "pattern";
    char filepath[256];

    int demand = 1024 * 1024;

    int testid = 1;

    /* First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
	for (int n = size/16; n <= size/2; n *= 2) 
	{
	    if (rank == 0)
	    {
		sprintf( filepath, "%s_first%d_last%d", base, m, n);
		optiq_pattern_firstm_lastn(filepath, size, demand, m, n, false);
	    }


	    if (rank == 0)
            {
		sprintf( filepath, "%s_first%d_last%d_random", base, m, n);
                optiq_pattern_firstm_lastn(filepath, size, demand, m, n, true);
            }

	}
    }

    /* Benchmark for subgroup aggregation */
    for (int i = 4; i < 128; i *= 2)
    {
        if (rank == 0)
        {
	    optiq_pattern_subgroup_agg(filepath, size, i, demand);
        }
    }

    /* Benchmark for overlapped groups */
    for (int m = size/16; m <= size/2; m *= 2)
    {
        for (int n = size/16; n <= size/2; n *= 2)
        {
	    for (int ov = 2; ov <= 4; ov *= 2) 
	    {
		if (rank == 0)
		{
		    int ovl = m/ov < n/ov ? m/ov : n/ov;

		    sprintf (filepath, "%s_overlap%d_%d", base, m, n);
		    optiq_pattern_overlap (filepath, size, demand, m, ovl, n, false);

		    sprintf (filepath, "%s_overlap%d_%d_random", base, m, n);
                    optiq_pattern_overlap (filepath, size, demand, m, ovl, n, true);
		}
	    }
        }
    }
    
    if (rank == 0) {
        printf("Finished benchmarking\n");
    }

    return 0;
}
