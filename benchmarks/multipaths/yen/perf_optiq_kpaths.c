#include "optiq.h"
#include "mpi.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();

    int demand = 1024 * 1024;

    if (argc > 1) {
	demand = atoi(argv[2]) * 1024;
    }

    //odp.print_local_jobs = true;
    char filepath[256];

    for (int i = 0; i < 16; i++)
    {
	sprintf(filepath, "test%d", i);

	optiq_execute_jobs_from_file (filepath, demand);

	opi.iters = 1;
	optiq_opi_collect();
	if (pami_transport->rank == 0) {
	    optiq_opi_print();
	}
    }

    if (pami_transport->rank == 0) {
        printf("Finished testing optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
