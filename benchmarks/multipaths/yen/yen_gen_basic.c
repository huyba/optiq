#include <stdio.h>
#include <stdlib.h>

#include "yen_gen_basic.h"

void gen_91_cases (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    int start_testid = 0;
    char jobfile[256];

    int size = topo->num_nodes * topo->num_ranks_per_node;

    /* Generate disjoint First m send data to last n */
    for (int m = size/16; m <= size/2; m *= 2)
    {
        for (int n = size/16; n <= size/2; n *= 2)
        {
            if (mintestid <= testid && testid <=maxtestid)
            {
                optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node,  true);

                /* Not allow to generate too many paths, leading to */
                int numpairs = m > n ? m : n;
                int maxpaths = numpaths;
                if (maxpathspertest / numpairs < maxpaths) {
                    maxpaths = maxpathspertest / numpairs;
                }

                sprintf(name, "Test No. %d Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, size-n, size -1, maxpaths);
                sprintf(jobs[0].name, "%s", name);
                sprintf(jobfile, "test%d", testid);

                search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);
            }

            testid++;
            jobs.clear();
        }
    }

    /* Overlap Generate paths */
    for (int m = size/16; m <= size/2; m *= 2)
    {
        for (int n = size/16; n <= size/2; n *= 2)
        {
            for (int l = m/8; l <= m/2; l *= 2)
            {
                if (mintestid <= testid && testid <=maxtestid)
                {
                    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, true);

                    /* Not allow to generate too many paths, leading to */
                    int numpairs = m > n ? m : n;
                    int maxpaths = numpaths;
                    if (maxpathspertest / numpairs < maxpaths) {
                        maxpaths = maxpathspertest / numpairs;
                    }

                    sprintf(name, "Test No. %d Overlap %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, m-l, n + m -l -1, maxpaths);

                    sprintf(jobs[0].name, "%s", name);
                    sprintf(jobfile, "test%d", testid);

                    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
                }
                testid++;
                jobs.clear();
            }
        }
    }

    /* Overlap Generate paths */
    for (int m = size/8; m <= size/2; m *= 2)
    {
        for (int n = m/16; n <= m/4; n *= 2)
        {
            for (int l = 0; l <= m/2; l += m/4)
            {
                if (mintestid <= testid && testid <=maxtestid)
                {
                    optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, l, topo->num_ranks_per_node, true);

                    /* Not allow to generate too many paths, leading to */
                    int numpairs = m > n ? m : n;
                    int maxpaths = numpaths;
                    if (maxpathspertest / numpairs < maxpaths) {
                        maxpaths = maxpathspertest / numpairs;
                    }

                    sprintf(name, "Test No. %d Overlap %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, l, n + l -1, maxpaths);

                    sprintf(jobs[0].name, "%s", name);
                    sprintf(jobfile, "test%d", testid);

                    search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
                }
                testid++;
                jobs.clear();
            }
        }
    }
}

void gen_distance_increase_2k4k(struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    int testid = 0;
    int start_testid = 0;
    char jobfile[256];

    int size = topo->num_nodes * topo->num_ranks_per_node;
    
    /* Distance increase for 2k and 4k */
    if (size >= 2048 && size <= 4096)
    {
        for (int m = size/32; m <= size/2; m *= 2)
        {
            for (int n = size/32; n <= size/2; n *= 2)
            {
                int min = m < n ? m : n;
                int t = 0;
                if (size/min <= 8) {
                    t = 1;
                } else {
                    t = size/min/4;;
                }

                for (int d = m; d < size - n; d += min * t)
                {
                    if (mintestid <= testid && testid <=maxtestid)
                    {
                        optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, d, topo->num_ranks_per_node, false);

                        /* Not allow to generate too many paths, leading to */
                        int numpairs = m > n ? m : n;
                        int maxpaths = numpaths;
                        if (maxpathspertest / numpairs < maxpaths) {
                            maxpaths = maxpathspertest / numpairs;
                        }

                        sprintf(name, "Test No. %d Disjoint Increasing distance %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, d, d + n - 1, maxpaths);

                        sprintf(jobs[0].name, "%s", name);
                        sprintf(jobfile, "test%d", testid);

                        search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);
                    }

                    testid++;
                    jobs.clear();
                }
            }
        }
    }
}
