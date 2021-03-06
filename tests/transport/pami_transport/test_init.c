#include <stdio.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    optiq_pami_transport_init();

    struct optiq_pami_transport* pami_transport = optiq_pami_transport_get();

    printf("rank %d size %d\n", pami_transport->rank, pami_transport->size);

    optiq_pami_transport_finalize();

    return 0;
}
