#include "optiq.h"
#include "mpi.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    int world_rank = pami_transport->rank;
    int world_size = pami_transport->size;

    char *filepath = "pattern";
    int demand = 1024 * 1024;

    if (argc > 1) {
	filepath = argv[1];
    }
    if (argc > 2) {
	demand = atoi(argv[2]);
    }

    if (world_rank == 0) {
	optiq_pattern_half_half(filepath, world_size, demand);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_mton_from_file(filepath);

    if (world_rank == 0) {
        printf("Finished benchmarking\n");
    }

    opi.iters = 1;
    optiq_opi_collect(world_rank);

    optiq_finalize();

    return 0;
}
