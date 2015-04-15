#include "optiq.h"
#include <mpi.h>

#include "optiq_benchmark.h"
#include "mpi_benchmark.h"

int main(int argc, char **argv)
{
    optiq_init(argc, argv);

    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *schedule = optiq_schedule_get();

    int rank = pami_transport->rank;
    int size = pami_transport->size;

    int demand = 1024 * 1024;
    int start = 0, end = 0;
    char *path;

    if (argc > 1) {
	path = argv[1];
    }

    if (argc > 2) {
	start = atoi(argv[2]);
    }

    if (argc > 3) {
	end = atoi(argv[3]);
    }

    if (argc > 4) {
	demand = atoi(argv[4]) * 1024;
    }

    char filepath[256];

    for (int i = start; i < end; i++)
    {
	sprintf(filepath, "%s/test%d", path, i);

	odp.print_local_jobs = true;
	odp.print_rput_rdone_notify_msg = true;
	odp.print_recv_rput_done_msg = true;
	odp.print_mem_exchange_status = true;
	odp.print_mem_adv_exchange_msg = true;

	for (int chunk = 8 * 1024; chunk <=  demand; chunk *= 2)
	{
	    schedule->chunk_size = chunk;
	    schedule->auto_chunksize = false;

	    odp.print_path_rank = true;
	    odp.print_mem_reg_msg =  true;
	    odp.print_mem_exchange_status = true;
            odp.print_mem_adv_exchange_msg = true;

	    odp.print_local_jobs = true;

	    //odp.print_mem_avail = true;

	    odp.print_rput_msg = true;
	    odp.print_rput_rdone_notify_msg = true;
            odp.print_recv_rput_done_msg = true;

	    optiq_benchmark_jobs_from_file (filepath, demand);

	    opi.iters = 1;
	    optiq_opi_collect();

	    if (rank == 0) 
	    {
		printf("chunk size = %d\n", chunk);
		optiq_opi_print();

		if (mpi_time > max_opi.transfer_time) 
		{
		    double mpi_bw = max_opi.recv_len / mpi_time / 1024 / 1024 * 1e6;
		    double optiq_bw = max_opi.recv_len / max_opi.transfer_time / 1024 / 1024 * 1e6;

		    printf("Bingo %d %d %8.0f %8.4f %8.0f %8.4f \n", demand, chunk, mpi_time, mpi_bw, max_opi.transfer_time, optiq_bw);
		}

		optiq_path_print_stat (opi.paths, size, topo->num_edges);
		optiq_opi_clear();
	    }
	}
    }

    if (pami_transport->rank == 0) {
        printf("Finished testing optiq_alltoallv\n");
    }

    optiq_finalize();

    return 0;
}
