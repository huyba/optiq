#include "optiq.h"
#include "mpi.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    char *filepath = "pattern";
    int demand = 1024 * 1024;

    if (argc > 1) {
	filepath = argv[1];
    }
    if (argc > 2) {
	demand = atoi(argv[2]);
    }

    if (pami_transport->rank == 0) {
	optiq_pattern_firstk_lastk(filepath, pami_transport->size, demand, pami_transport->size/2);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_mton_from_file(filepath);

    //if (world_rank == 0) {
        printf("Finished testing optiq_alltoallv\n");
    //}

    opi.iters = 1;
    optiq_opi_collect();
    if (pami_transport->rank == 0) {
	optiq_opi_print();
    }

    optiq_finalize();

    return 0;
}
