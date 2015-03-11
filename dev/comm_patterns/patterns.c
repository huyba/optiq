#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>

#include "patterns.h"

void optiq_patterns_read_from_file(char *filename, std::vector<std::pair<std::pair<int, int>, int > > &requests)
{
    std::ifstream infile(filename);

    int source, dest, demand;

    while (infile >> source >> dest >> demand)
    {
	std::pair<int, int> sd = std::make_pair(source, dest);
	std::pair<std::pair<int, int>, int> ssd = std::make_pair(sd, demand);
	requests.push_back(ssd);
    }

    infile.close();
}

void optiq_patterns_convert_from_ssd_to_mpialltoallv(std::vector<std::pair<std::pair<int, int>, int > > &requests, int *sendcounts, int *recvcounts, int rank)
{
    int source, dest, demand;

    for (int i = 0; i < requests.size(); i++)
    {
	source = requests[i].first.first;
	dest = requests[i].first.second;
	demand = requests[i].second;

	if (source == rank) {
	    sendcounts[dest] = demand;
	}

	if (dest == rank) {
	    recvcounts[source] = demand;
	}
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

void optiq_pattern_half_half(char *filepath, int num_ranks, int demand)
{
    std::ofstream file;
    file.open(filepath);

    for (int i = 0; i < num_ranks / 2; i++) {
	file << i << " " << (i + num_ranks / 2) << " " << demand << std::endl;
    }
 
    file.close();
}
