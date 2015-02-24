#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "patterns.h"

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
