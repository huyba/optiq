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
	demand = atoi(argv[2]) * 1024;
    }

    //odp.print_local_jobs = true;

    optiq_execute_jobs_from_file (filepath, demand);

    if (pami_transport->rank == 0) {
        printf("Finished testing optiq_alltoallv\n");
    }

    opi.iters = 1;
    optiq_opi_collect();
    if (pami_transport->rank == 0) {
	optiq_opi_print();
    }

    optiq_finalize();

    return 0;
}
