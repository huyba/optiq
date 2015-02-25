#include "optiq.h"

int main(int argc, char **argv)
{
    optiq_init();

    if (pami_transport->rank == 0)
    {
	optiq_topology_print(topo);

	optiq_multibfs_print();

	optiq_pami_transport_print();
    }

    optiq_finalize();

    return 0;
}
