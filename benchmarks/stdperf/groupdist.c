#include <optiq.h>
#include <vector>
#include <algorithm>

int main(int argc, char **argv)
{
    int size[5];
    int num_dims = 5;

    size[0] = atoi (argv[1]);
    size[1] = atoi (argv[2]);
    size[2] = atoi (argv[3]);
    size[3] = atoi (argv[4]);
    size[4] = atoi (argv[5]);

    int s1 = atoi (argv[6]);
    int e1 = atoi (argv[7]);
    int s2 = atoi (argv[8]);
    int e2 = atoi (argv[9]);

    std::vector<struct job> jobs;
    jobs.clear();

    topo = (struct optiq_topology *) calloc (1, sizeof (struct optiq_topology));   

    optiq_topology_init_with_params(num_dims, size, topo);

    topo->num_ranks_per_node = 1;
    int demand = 1;
    optiq_pattern_m_to_n_to_jobs (jobs, topo->num_nodes, demand, e1-s1+1, s1, e2-s2+1, s2, topo->num_ranks_per_node, false);

    std::vector<int> hops;

    double total = 0;
    for (int i = 0; i < jobs.size(); i++)
    {
        int hop = optiq_topology_get_hop_distance(jobs[i].source_rank, jobs[i].dest_rank);
        total += hop;
        hops.push_back(hop);
    }

    std::sort(hops.begin(), hops.end());

    printf("%8.0f %d %d %4.2f %d\n", total, hops[hops.size()-1], hops[0], total/hops.size(), hops[hops.size()/2]);

    return 0;
}
