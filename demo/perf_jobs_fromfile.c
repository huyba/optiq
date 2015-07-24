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

    int demand = 8* 1024 * 1024;
    int mindemand = demand;
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
	mindemand = atoi(argv[4]) * 1024;
    }

    if (argc > 5) {
	demand = atoi (argv[5]) * 1024;
    }

    int minchunksize = 32 * 1024;
    int maxchunksize = 128 * 1024;

    if (argc > 6) {
        minchunksize = atoi (argv[6]) * 1024;
    }

    if (argc > 7) {
        maxchunksize = atoi (argv[7]) * 1024;
    }

    char filepath[256];

    if (rank == 0) {
	printf("MPI/OPTIQ msg chunk totalsize time(us) bandwidth(MB/s) #path totalhops totalloads loadlinks maxhops minhops avghops medhops maxload minload avgload medload\n\n");
    }

    for (int nbytes = demand; nbytes >= mindemand; nbytes /= 2)
    {
	for (int i = start; i <= end; i++)
	{
            schedule->test_id = i;

	    sprintf(filepath, "%s/test%d", path, i);

	    if (rank == 0) {
		printf("Test No. %d\n", i);
	    }

            int minchunk = minchunksize;
            if (nbytes < minchunk) {
                minchunk = nbytes;
            }

            int maxchunk = maxchunksize;
            if (nbytes < maxchunk) {
                maxchunk = nbytes;
            }

	    for (int chunk = minchunk; chunk <=  maxchunk; chunk *= 2)
	    {
		schedule->chunk_size = chunk;
		schedule->auto_chunksize = false;

		odp.collect_transport_perf = true;
		odp.print_transport_perf = true;
		//odp.print_mpi_paths = true;

		//odp.print_done_status = true;

		//odp.print_path_rank = true;
		//odp.print_job = true;
		//odp.print_mem_reg_msg =  true;
		//odp.print_mem_exchange_status = true;
		//odp.print_mem_adv_exchange_msg = true;

		//odp.print_local_jobs = true;

		//odp.print_mem_avail = true;

		//odp.print_rput_msg = true;
		//odp.print_rput_rdone_notify_msg = true;
		//odp.print_recv_rput_done_msg = true;
		//odp.print_pami_transport_status = true;

		bool ret = optiq_benchmark_jobs_from_file (filepath, nbytes);

                if (!ret) {
                    break;
                }

		opi.iters = 1;
		optiq_opi_collect();

		if (rank == 0) 
		{
		    sprintf(opi.prefix, "%s", "O");
		    opi.test_id = i;
		    sprintf(opi.name, "%s", "OPTIQ_Alltoallv");
		    opi.message_size = nbytes;
		    opi.chunk_size = chunk;

		    optiq_path_compute_stat (opi.paths, size, topo->num_edges);
		    optiq_opi_compute_stat ();

		    optiq_opi_print();
		}

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
