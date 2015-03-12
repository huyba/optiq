#include <mpi.h>

#include "optiq.h"
#include "optiq_benchmark.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Need file path as the first argumement param\n");
	return 0;
    }

    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    char *filepath = argv[1];

    optiq_benchmark_pattern_from_file (filepath, rank, size);

    optiq_finalize();

    return 0;
}
