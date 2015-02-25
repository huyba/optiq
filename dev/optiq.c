#include "optiq.h"

void optiq_init()
{
    optiq_topology_init();
    optiq_pami_transport_init();
    optiq_multibfs_init();
}

void optiq_finalize()
{
    optiq_topology_finalize();
    optiq_pami_transport_finalize();
    optiq_multibfs_finalize();
}
