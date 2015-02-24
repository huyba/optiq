#include "optiq.h"

void optiq_init()
{
    optiq_topology_init();
    optiq_pami_transport_init();
}
