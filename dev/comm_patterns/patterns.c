#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>

#include "util.h"
#include "patterns.h"

void optiq_patterns_read_requests_from_file(char *filename, std::vector<struct job> &jobs)
{
    std::ifstream infile(filename);

    int source, dest, demand;
    int id = 0;

    while (infile >> source >> dest >> demand)
    {
	struct job newjob;

	newjob.job_id = id;
	newjob.source_id = source;
	newjob.dest_id = dest;
	newjob.demand = demand;

	jobs.push_back(newjob);
	id++;
    }

    infile.close();
}

void optiq_patterns_convert_requests_to_sendrecvcounts(std::vector<struct job> &jobs, int* sendcounts, int* recvcounts, int rank)
{
    int source, dest, demand;

    for (int i = 0; i < jobs.size(); i++)
    {
	source = jobs[i].source_id;
	dest = jobs[i].dest_id;
	demand = jobs[i].demand;

	if (source == rank) {
	    sendcounts[dest] = demand;
	}

	if (dest == rank) {
	    recvcounts[source] = demand;
	}
    }
}

void optiq_patterns_alltoallv_from_file(char *filepath, std::vector<struct job> &jobs, void * &sendbuf, int * &sendcounts, int * &sdispls, void * &recvbuf, int * &recvcounts, int* &rdispls, int rank, int size)
{
    optiq_patterns_read_requests_from_file(filepath, jobs);

    sendcounts = (int *) calloc (1, sizeof (int) * size);
    recvcounts = (int *) calloc (1, sizeof (int) * size);

    optiq_patterns_convert_requests_to_sendrecvcounts(jobs, sendcounts, recvcounts, rank);

    sdispls = (int *) calloc (1, sizeof(int) * size);
    rdispls = (int *) calloc (1, sizeof(int) * size);

    int sbytes = 0;
    for (int i = 0; i < size; i++)
    {
        if (sendcounts[i] != 0)
        {
            sdispls[i] = sbytes;
            sbytes += sendcounts[i];
        }
    }

    if (sbytes > 0) {
        sendbuf = malloc (sbytes);
    }

    int rbytes = 0;
    for (int i = 0; i < size; i++)
    {
        if (recvcounts[i] != 0)
        {
            rdispls[i] = rbytes;
            rbytes += recvcounts[i];
        }
    }

    if (rbytes > 0) {
        recvbuf = malloc (rbytes);
    }
}


void patterns_disjoint_contigous_multiranks_multinodes(int *sendcounts, int *recvcounts, int rank, int num_ranks, int num_ranks_per_node, int nbytes)
{
    memset(sendcounts, 0, sizeof(int) * num_ranks);
    memset(recvcounts, 0, sizeof(int) * num_ranks);

    int destrank = 0, sourcerank = 0;

    int pos = rank % num_ranks_per_node;
    int quarter = num_ranks_per_node / 4;

    if (rank < num_ranks/2) 
    {
	if (pos < quarter)
	{
	    destrank = (rank + num_ranks / 2 - quarter) % (num_ranks / 2) + num_ranks / 2;
	}
	else if (quarter <= pos && pos < quarter * 3)
	{
	    destrank = rank + num_ranks / 2;
	}
	else if (quarter * 3 <= pos)
	{
	    destrank = (rank + num_ranks / 2 + quarter) % (num_ranks / 2) + num_ranks / 2;
	}

	sendcounts[destrank] = nbytes;

	/*printf("Rank %d dest is %d\n", rank, destrank);*/
    }

    if (rank >= num_ranks/2)
    {
	if (pos < quarter)
        {
            sourcerank = (rank - num_ranks / 2 - quarter) % (num_ranks / 2);
	    if (sourcerank < 0) {
		sourcerank += num_ranks / 2;
	    }
        }
        else if (quarter <= pos && pos < 3 * quarter)
        {
            sourcerank = rank - num_ranks / 2;
        }   
        else if (3 * quarter <= pos)
        {
            sourcerank = (rank - num_ranks / 2 + quarter) % (num_ranks / 2);
        }

	recvcounts[sourcerank] = nbytes;

	/*printf("Rank %d source is %d\n", rank, sourcerank);*/
    }
}

/*The first k nodes communicate with the last k nodes. 1-1*/
void disjoint_contiguous_firstk_lastk(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int k)
{
    sources.clear();
    dests.clear();
    source_dests.clear();

    if (k > num_nodes) {
        return;
    }

    for (int i = 0; i < k; i++) {
	sources.push_back(i);
    }

    for (int i = num_nodes - k; i < num_nodes; i++) 
    {
	dests.push_back(i);
	std::vector<int> d;
        d.push_back (i);
        std::pair<int, std::vector<int> > p = make_pair (i + k - num_nodes, d);
	source_dests.push_back(p);
    }
}

void disjoint_contiguous (int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio)
{
    if (! (ratio == 1 || ratio == 3 || ratio == 7)) {
	printf("Not accepted ratio\n");
	exit(0);
    }

    sources.clear();
    dests.clear();
    source_dests.clear();

    for (int i = num_nodes / (ratio+1) * ratio; i < num_nodes; i++) {
        dests.push_back(i);
    }

    for (int i = 0; i < num_nodes / (ratio + 1) * ratio; i++) 
    {
	sources.push_back (i);
	std::vector<int> nd;
	nd.push_back (i/ratio + num_nodes/(ratio+1)*ratio);
	std::pair<int, std::vector<int> > p = make_pair (i, nd);
	source_dests.push_back (p);
    }
}

void subset_udistributed_ioagg(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio)
{
    sources.clear();
    dests.clear();
    source_dests.clear();

    for (int i = 0; i < num_nodes/ratio; i++) {
        dests.push_back(i * ratio + ratio/2);
    }

    for (int i = 0; i < num_nodes; i++) {
        sources.push_back (i);
        std::vector<int> nd;
        nd.push_back (i/ratio + ratio/2);
        std::pair<int, std::vector<int> > p = make_pair (i, nd);
        source_dests.push_back (p);
    }
}

void subset_rdistributed(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio)
{
    sources.clear();
    dests.clear();
    source_dests.clear();

    srand(time(NULL));

    while (dests.size() < num_nodes/ratio) {
	int dest = rand() % num_nodes;
	bool existing = false;

	for (int i = 0; i < dests.size(); i++) {
	    if (dests[i] == dest) {
		existing = true;
		break;
	    }
	}

	if (!existing) {
	    dests.push_back(dest);
	}
    }

    for (int i = 0; i < num_nodes; i++) {
        sources.push_back (i);
        std::vector<int> nd;
        nd.push_back (dests[i/ratio]);
        std::pair<int, std::vector<int> > p = make_pair (i, nd);
        source_dests.push_back (p);
    }
}

void overlap_contiguous(int num_nodes, std::vector<int> &sources, std::vector<int> &dests, std::vector<std::pair<int, std::vector<int> > > &source_dests, int ratio)
{
    sources.clear();
    dests.clear();
    source_dests.clear();

    if (ratio == 1) {
	for (int i = num_nodes/8 * 2; i < num_nodes; i++) {
	    dests.push_back(i);
	}

	for (int i = 0; i < num_nodes/8 * 6; i++) {
	    sources.push_back(i);
	    std::vector<int> d;
	    d.push_back(i + num_nodes/8 * 2);
	    std::pair<int, std::vector<int> > p = make_pair (i, d);
	    source_dests.push_back (p);
	}
    }
    else if (ratio == 3) {
	for (int i = num_nodes/8 * 5; i < num_nodes/8 * 7; i++) {
            dests.push_back(i);
        }

        for (int i = 0; i < num_nodes/8 * 6; i++) {
            sources.push_back(i);
            std::vector<int> d;
            d.push_back(i/3 + num_nodes/8 * 6);
            std::pair<int, std::vector<int> > p = make_pair (i, d);
            source_dests.push_back (p);
        }
    }
    else {
	printf("Not acceptable ration, only 1 or 3\n");
	exit(0);
    }
}

void optiq_pattern_firstk_lastk(char *filepath, int num_ranks, int demand, int k)
{
    std::ofstream file;
    file.open(filepath);

    for (int i = 0; i < k; i++) {
        file << i << " " << (num_ranks -k + i) << " " << demand << std::endl;
    }

    file.close();
}

void optiq_pattern_lastk_firstk(char *filepath, int num_ranks, int demand, int k)
{
    std::ofstream file;
    file.open(filepath);

    for (int i = 0; i < k; i++) {
        file << (num_ranks -k + i) << " " << i << " " << demand << std::endl;
    }

    file.close();
}

void optiq_pattern_firstm_lastn_to_jobs (std::vector<struct job> &jobs, int numranks, int demand, int m, int n)
{
    jobs.clear();
    int jobid = 0;

    if (m > n) 
    {
	int r = m/n;
	int d = numranks - n;

	for (int i = 0; i < m; i += r) 
	{
	    for (int j = 0; j < r; j++) 
	    {
		struct job new_job;
		new_job.job_id = jobid;
		new_job.source_rank = i + j;
		new_job.source_id = i + j;
		new_job.dest_rank = d;
		new_job.dest_id = d;
		new_job.demand = demand;

		jobs.push_back(new_job);
		jobid++;
	    }
	    d++;
	}
    } 
    else 
    {
	int r = n/m;
        int d = numranks - n;

        for (int i = 0; i < m; i++)     
        {
            for (int j = 0; j < r; j++) 
            {
		struct job new_job;
		new_job.job_id = jobid;
		new_job.source_rank = i;
		new_job.source_id = i;
		new_job.dest_rank = d + j;
		new_job.dest_id = d + j;
		new_job.demand = demand;

		jobs.push_back(new_job);
		jobid++;
            }
            d += r;
        }
    }
}

void optiq_pattern_firstm_lastn(char *filepath, int numranks, int demand, int m, int n, bool random)
{
    std::ofstream file;
    file.open(filepath);

    std::vector<std::pair<int, int> > source_dests;
    source_dests.clear();

    if (m > n) 
    {
	int r = m/n;
	int d = numranks - n;

	for (int i = 0; i < m; i += r) 
	{
	    for (int j = 0; j < r; j++) 
	    {
		std::pair<int, int> sd = std::make_pair (i + j, d);
		source_dests.push_back(sd);
	    }
	    d++;
	}
    } 
    else 
    {
	int r = n/m;
        int d = numranks - n;

        for (int i = 0; i < m; i++)     
        {
            for (int j = 0; j < r; j++) 
            {
		std::pair<int, int> sd = std::make_pair (i, d + j);
		source_dests.push_back(sd);
            }
            d += r;
        }
    }

    if (random) {
	optiq_util_randomize_source_dests (source_dests);
    }

    for (int i = 0; i < source_dests.size(); i++)
    {
	file << source_dests[i].first  << " " << source_dests[i].second << " " << demand << std::endl;
    }

    file.close();
}

void optiq_pattern_subgroup_agg (char *filepath, int numranks, int subgroupsize, int demand)
{
    std::ofstream file;
    file.open (filepath);

    for (int i = 0; i < numranks; i++) {
	file << i  << " " << (i/subgroupsize) * subgroupsize + subgroupsize/2  << " " << demand << std::endl;
    }

    file.close();
}

void optiq_pattern_m_to_n(char *filepath, int numranks, int demand, int m, int startm, int n, int startn, bool random)
{
    std::vector<std::pair<int, int> > source_dests;
    source_dests.clear();

    std::ofstream file;
    file.open(filepath);

    if (m > n)
    {
        int r = m/n;
        int d = startn;

        for (int i = startm; i < m + startm; i += r)
        {
            for (int j = 0; j < r; j++)
            {
		std::pair<int, int> sd = std::make_pair (i + j, d);
                source_dests.push_back(sd);
            }
            d++;
        }
    }
    else
    {
        int r = n/m;
        int d = startn;

        for (int i = startm; i < m +  startm; i++)
        {
            for (int j = 0; j < r; j++)
            {
		std::pair<int, int> sd = std::make_pair (i, d + j);
                source_dests.push_back(sd);
            }
            d += r;
        }
    }

    if (random) {
        optiq_util_randomize_source_dests (source_dests);
    }

    for (int i = 0; i < source_dests.size(); i++)
    {
        file << source_dests[i].first  << " " << source_dests[i].second << " " << demand << std::endl;
    }

    file.close();   
}

void optiq_pattern_overlap (char *filepath, int numranks, int demand, int m, int numoverlap, int n, bool random)
{
    optiq_pattern_m_to_n(filepath, numranks, demand, m, 0, n, m - numoverlap, random);
}


void optiq_pattern_m_to_n_to_vectors (int m, int startm, int n, int startn, std::vector<std::pair<int, std::vector<int> > > &source_dests)
{
    source_dests.clear();

    printf("%d ranks from %d to %d talks to %d rank from %d to %d\n", m, startm, startm + m - 1, n, startn, startn + n - 1);

    if (m > n)
    {
        int r = m/n;
        int d = startn;

        for (int i = startm; i < m + startm; i += r)
        {
            for (int j = 0; j < r; j++)
            {
		std::vector<int> dests;
		dests.clear();
		dests.push_back (d);

		std::pair<int, std::vector<int> > sd = std::make_pair (i + j, dests);
		source_dests.push_back (sd);
	    }

            d++;
        }
    }
    else
    {
        int r = n/m;
        int d = startn;

        for (int i = startm; i < m +  startm; i++)
        {
	    std::vector<int> dests;
	    dests.clear();

            for (int j = 0; j < r; j++)
            {
		 dests.push_back (d + j);
            }

	    std::pair<int, std::vector<int> > sd = std::make_pair (i, dests);
	    source_dests.push_back (sd);

            d += r;
        }
    }
}
