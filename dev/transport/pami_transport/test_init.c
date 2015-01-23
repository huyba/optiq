#include <stdio.h>

#include "pami_transport.h"

int main(int argc, char **argv)
{
    pami_transport_init();

    struct optiq_pami_transport* pami_transport = get_pami_transport();

    printf("rank %d size %d\n", pami_transport->rank, pami_transport->size);

    return 0;
}
