#include "optiq.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_size = pami_transport->size;
    int world_rank = pami_transport->rank;

    std::vector<struct path *> complete_paths;
    std::vector<std::pair<int, std::vector<int> > > notify_list;
    std::vector<std::pair<int, std::vector<int> > > intermediate_notify_list;
    int num_alive_flows;

    int num_rank = atoi(argv[1]);

    struct path p;
    for (int i = 0; i < num_rank-1; i++) {
	struct arc a;
	a.u = i;
	a.v = i+1;
	p.arcs.push_back(a);
    }

    complete_paths.push_back(&p);

    for (int rank = 0; rank < num_rank; rank++)
    {
	build_notify_lists(complete_paths, notify_list, intermediate_notify_list, num_alive_flows, rank);

	printf("Rank %d, num_alive_flows = %d\n", rank, num_alive_flows);

	for (int i = 0; i < notify_list.size(); i++) 
	{
	    printf("Notify list: ");
	    for (int j = 0; j < notify_list[i].second.size(); j++)
	    {
		printf("%d ", notify_list[i].second[j]);
	    }
	    printf("\n");
	}

	for (int i = 0; i < intermediate_notify_list.size(); i++)
        {
            printf("Notify list: ");
            for (int j = 0; j < intermediate_notify_list[i].second.size(); j++)
            {
                printf("%d ", intermediate_notify_list[i].second[j]);
            }
            printf("\n");
        }
    }

    optiq_finalize();

    return 0;
}
