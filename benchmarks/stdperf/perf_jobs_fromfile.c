#include "optiq.h"
#include "mpi.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int demand = 1024 * 1024;
    char *path;

    if (argc > 1) {
	path = argv[1];
    }
    if (argc > 2) {
	demand = atoi(argv[2]) * 1024;
    }

    //odp.print_path_rank = true;
    //odp.print_local_jobs = true;
    char filepath[256];

    schedule->auto_chunksize = false;
    for (int i = 0; i < 7; i++)
    {
	sprintf(filepath, "%s/test%d", path, i);

	//for (int chunk = 4 * 1024; chunk <=  demand; chunk *= 2)
	//{
	    //schedule->chunk_size = chunk;
	    optiq_execute_jobs_from_file (filepath, demand);

	    opi.iters = 1;
	    optiq_opi_collect();

	    if (rank == 0) 
	    {
		//printf("chunk size = %d\n", chunk);
		optiq_opi_print();
		optiq_path_print_stat (opi.paths, size, topo->num_edges);
		optiq_opi_clear();
	    }
	//}
    }

    if (pami_transport->rank == 0) {
        printf("Finished testing optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
