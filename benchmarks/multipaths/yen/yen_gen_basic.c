#include <stdio.h>
#include <stdlib.h>
#include <time.h> 

#include "yen_gen_basic.h"

int maxpathspertest = 50 * 1024;

int maxtestid, mintestid;

void search_and_write_to_file (std::vector<struct job> &jobs, char*jobfile, char *graphFilePath, int num_paths)
{
    int rank, size;

    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &size);
    char pairfile[256];

    for (int i = 0; i < jobs.size(); i++)
    {
        if (rank == i % size)
        {
            optiq_alg_yen_k_shortest_paths_job (graphFilePath, jobs[i], num_paths);

            sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
            optiq_job_write_to_file (jobs, pairfile);

            /*free paths*/
            for (int j = 0; j < jobs[i].paths.size(); j++)
            {
                jobs[i].paths[j]->arcs.clear();
                free (jobs[i].paths[j]);
            }
            jobs[i].paths.clear();
        }
    }
}

void optiq_job_read_jobs_from_cesm (std::vector<struct job> &jobs, int datasize, char *cesmFilePath)
{
    char cesmfile[2048];

    int job_id = 0;

    for (int i = mintestid; i <= maxtestid; i++)
    {
        sprintf(cesmfile, "%s2xRearr.%05d", cesmFilePath, i);

        FILE * fp;
        char line[256];

        if( access( cesmfile, F_OK ) == -1 )
        {
            return;
        }

        fp = fopen(cesmfile, "r");

        if (fp == NULL) {
            return;
        }

        int source_rank, dest_rank, num_points;

        char temp[256], name[256];
        bool exist;

        while (fgets(line, 80, fp) != NULL)
        {
            if (line[1] == 'S')
            {
                fgets(line, 80, fp);

                while (line[1] != 'R')
                {
                    trim(line);
                    sscanf(line, "%d %d %d", &source_rank, &dest_rank, &num_points);
                    /*printf("job_id = %d source_rank = %d, dest_rank = %d\n", job_id, source_rank, dest_rank);*/

                    struct job new_job;
                    new_job.job_id = job_id;
                    new_job.source_id = 0;
                    new_job.source_rank = source_rank;
                    new_job.dest_id = 0;
                    new_job.dest_rank = dest_rank;
                    new_job.demand = num_points * datasize;
                    job_id++;

                    jobs.push_back(new_job);

                    fgets(line, 80, fp);
                }

                if (line[1] == 'R')
                {
                    while (fgets(line, 80, fp) != NULL)
                    {
                        trim(line);
                        sscanf(line, "%d %d %d", &dest_rank, &source_rank, &num_points);
                        /*printf("job_id = %d source_rank = %d, dest_rank = %d\n", job_id, source_rank, dest_rank);*/

                        bool exist = false;
                        for (int i = 0; i < jobs.size(); i++)
                        {
                            if (jobs[i].source_rank == source_rank && jobs[i].dest_rank == dest_rank)
                            {
                                exist = true;
                                break;
                            }
                        }

                        if (!exist)
                        {
                            struct job new_job;
                            new_job.job_id = job_id;
                            new_job.source_id = 0;
                            new_job.source_rank = source_rank;
                            new_job.dest_id = 0;
                            new_job.dest_rank = dest_rank;
                            new_job.demand = num_points * datasize;
                            job_id++;

                            jobs.push_back(new_job);
                        }
                    }
                }

                break;
            }
        }

        fclose(fp);

    }
}

void gen_paths_cesm (struct optiq_topology *topo, int datasize, char *graphFilePath, int numpaths, char *cesmFilePath, bool gather)
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

    jobs.clear();

    /* Subset Generate paths*/
    optiq_job_read_jobs_from_cesm (jobs, datasize, cesmFilePath);

    for (int i = 0; i < jobs.size(); i++)
    {
        jobs[i].source_id = jobs[i].source_rank/topo->num_ranks_per_node;
        jobs[i].dest_id = jobs[i].dest_rank/topo->num_ranks_per_node;
    }

    /*If gather job from rank to node*/
    if (gather)
    {
        for (int i = 0; i < jobs.size(); i++)
        {
            for (int j = 0; j < i; j++)
            {
                if (jobs[i].source_id == jobs[j].source_id && jobs[i].dest_id == jobs[j].dest_id)
                {
                    jobs[j].demand += jobs[i].demand;
                    jobs.erase (jobs.begin() + i);
                    i--;
                }
            }
        }

        for (int i = 0; i < jobs.size(); i++)
        {
            jobs[i].source_rank = jobs[i].source_id;
            jobs[i].dest_rank = jobs[i].dest_id;
        }
    }

    int maxpaths = numpaths;

    /*if (maxpaths > maxpathspertest/jobs.size())
      {
      maxpaths = maxpathspertest/jobs.size();
      }*/

    sprintf(name, "Test %d number of jobs, with %d paths per job", jobs.size(), maxpaths);
    sprintf(jobs[0].name, "%s", name);
    sprintf(jobfile, "test%d", maxpaths);

    if (rank == 0)
    {
        optiq_job_print_jobs(jobs);
    }

    search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);

    testid++;
    jobs.clear();
}



void gen_multiranks (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    char jobfile[256];

    int size = topo->num_nodes * topo->num_ranks_per_node;

    srand (time(NULL));

    /* Generate disjoint First m send data to last n */
    int m = 16;
    int n = 16;

    for (int m = 16; m <= 128; m *= 2)
    {
        for (int n = 16; n <= 128; n *= 2)
        {
            if (mintestid <= testid && testid <=maxtestid)
            {
                optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size/2, topo->num_ranks_per_node,  randompairing);

                if (demand == 0)
                {
                    for (int i = 0; i < jobs.size(); i++)
                    {
                        jobs[i].demand = (rand() + minsize) % maxsize;
                    }
                }

                /* Not allow to generate too many paths, leading to */
                int numpairs = m > n ? m : n;
                int maxpaths = numpaths;
                if (maxpathspertest / numpairs < maxpaths) {
                    maxpaths = maxpathspertest / numpairs;
                }

                sprintf(name, "Test No. %d Disjoint %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, 0, m-1, n, size/2, n-1, maxpaths);
                sprintf(jobs[0].name, "%s", name);
                sprintf(jobfile, "test%d", testid);

                search_and_write_to_file (jobs, jobfile, graphFilePath, maxpaths);
            }

            testid++;
            jobs.clear();
        }
    }
}

void gen_1_16_to_1_2 (struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize, int &testid, int demand, bool randompairing)
{
    int rank, numranks;
    MPI_Comm_rank (MPI_COMM_WORLD, &rank);
    MPI_Comm_size (MPI_COMM_WORLD, &numranks);

    std::vector<struct job> jobs;
    char name[256];
    char jobfile[256];

    int size = topo->num_nodes * topo->num_ranks_per_node;

    srand (time(NULL));

    /* Generate disjoint First m send data to last n */
    int m = size/16;
    int n = size/2;

    if (mintestid <= testid && testid <=maxtestid)
    {
        optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node,  randompairing);

        if (demand == 0)
        {
            for (int i = 0; i < jobs.size(); i++)
            {
                jobs[i].demand = (rand() + minsize) % maxsize;
            }
        }

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

    /* Overlap Generate paths */
    for (int l = m/8; l <= m/2; l *= 2)
    {
        if (mintestid <= testid && testid <=maxtestid)
        {
            optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, randompairing);

            if (demand == 0)
            {
                for (int i = 0; i < jobs.size(); i++)
                {
                    jobs[i].demand = (rand() + minsize) % maxsize;
                }
            }

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

    /* Subset Generate paths */
    m = size/16;
    n = size/2;

    for (int l = 0; l <= n/2; l += n/4)
    {
        if (mintestid <= testid && testid <=maxtestid)
        {
            optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, l, n, 0, topo->num_ranks_per_node, randompairing);

            if (demand == 0)
            {
                for (int i = 0; i < jobs.size(); i++)
                {
                    jobs[i].demand = (rand() + minsize) % maxsize;
                }
            }

            /* Not allow to generate too many paths, leading to */
            int numpairs = m > n ? m : n;
            int maxpaths = numpaths;
            if (maxpathspertest / numpairs < maxpaths) {
                maxpaths = maxpathspertest / numpairs;
            }

            sprintf(name, "Test No. %d Subset %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, l, m+l-1, n, 0, n - 1, maxpaths);

            sprintf(jobs[0].name, "%s", name);
            sprintf(jobfile, "test%d", testid);

            search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
        }
        testid++;
        jobs.clear();
    }
}

void gen_paths_with_rand_msg(struct optiq_topology *topo, char *graphFilePath, int numpaths, int minsize, int maxsize)
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
    int demand = 0;

    srand (time(NULL));

    /* Generate disjoint First m send data to last n */
    int m = size/16;
    int n = size/2;

    if (mintestid <= testid && testid <=maxtestid)
    {
        optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, size-n, topo->num_ranks_per_node,  false);

        for (int i = 0; i < jobs.size(); i++)
        {
            jobs[i].demand = (rand() + minsize) % maxsize;
        }

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

    /* Overlap Generate paths */
    for (int l = m/8; l <= m/2; l *= 2)
    {
        if (mintestid <= testid && testid <=maxtestid)
        {
            optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, 0, n, m - l, topo->num_ranks_per_node, true);

            for (int i = 0; i < jobs.size(); i++)
            {
                jobs[i].demand = (rand() + minsize) % maxsize;
            }

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

    /* Subset Generate paths */
    m = size/16;
    n = size/2;

    for (int l = 0; l <= n/2; l += n/4)
    {
        if (mintestid <= testid && testid <=maxtestid)
        {
            optiq_pattern_m_to_n_to_jobs (jobs, size, demand, m, l, n, 0, topo->num_ranks_per_node, true);

            for (int i = 0; i < jobs.size(); i++)
            {
                jobs[i].demand = (rand() + minsize) % maxsize;
            }

            /* Not allow to generate too many paths, leading to */
            int numpairs = m > n ? m : n;
            int maxpaths = numpaths;
            if (maxpathspertest / numpairs < maxpaths) {
                maxpaths = maxpathspertest / numpairs;
            }

            sprintf(name, "Test No. %d Subset %d ranks from %d to %d send data to %d ranks from %d to %d total %d paths", testid, m, l, m+l-1, n, 0, n - 1, maxpaths);

            sprintf(jobs[0].name, "%s", name);
            sprintf(jobfile, "test%d", testid);

            search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);
        }
        testid++;
        jobs.clear();
    }

    /*CESM-like pattern 87.5% of nodes communicate, 1 source talks to 3 dests, arbitrary selected, data size is also arbitrary selected*/
    {
        int job_id = 0;

        int num_nodes = size * 0.875;
        int num_pairs = 3;
        for (int source = 0; source < num_nodes; source++)
        {
            for (int i = 0; i < num_pairs; i++)
            {
                int dest = rand() % num_nodes;
                int demand = (rand() % 28) * 64 * 4096;

                struct job new_job;
                new_job.job_id = job_id;
                new_job.source_id = source;
                new_job.source_rank = source;
                new_job.dest_id = dest;
                new_job.dest_rank = dest;
                new_job.demand = demand;
                jobs.push_back(new_job);

                job_id++;
            }
        }

        /* Not allow to generate too many paths, leading to */
        int numpairs = num_nodes * num_pairs;
        int maxpaths = numpaths;
        if (maxpathspertest / numpairs < maxpaths) {
            maxpaths = maxpathspertest / numpairs;
        }

        sprintf(name, "Test No. %d CESM-like pattern %d ranks from %d to %d send data within the group with %d pairs per node with %d maxpaths", testid, num_nodes, 0, num_nodes-1, num_pairs, maxpaths);

        sprintf(jobs[0].name, "%s", name);
        sprintf(jobfile, "test%d", testid);

        search_and_write_to_file (jobs, jobfile, graphFilePath, numpaths);

        testid++;
        jobs.clear();
    }
}

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
