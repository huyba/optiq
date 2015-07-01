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

    /*Write a graph file*/
    char graphFilePath[] = "graph";

    if (rank == 0)
    {
        optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /*Read the demand file*/
    std::vector<struct job> jobs;
    jobs.clear();
    char rankdemandfile[256];

    int num_ranks_per_node = 8;
    int job_id = 0;

    for (int i = 0; i < 4; i++)
    {
	sprintf(rankdemandfile, "%s_%d", path, i);
	optiq_jobs_read_rank_demand(rankdemandfile, jobs, ion, num_ranks_per_node, job_id)
    }

    /*Add the IO bridge node as the destination and generate paths, model files*/
    char pairfile[256], jobfile[256];
    sprintf(jobfile, "test0");
    
    for (int i = 0; i < jobs.size(); i++)
    {
	if (jobs[i].source_id == rank)
	{
	    jobs[i].dest_id = optiq->bridge_id;

	    optiq_alg_yen_k_shortest_paths_job (graphFilePath, jobs[i], num_paths);

	    sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
            optiq_job_write_to_file (jobs, pairfile);

            /*free paths*/
            for (int j = 0; j < jobs[i].paths.size(); j++)
            {
                jobs[i].paths[j]->arcs.clear();
                free (jobs[i].paths[j]);
            }
            jobs[i].paths.clear();
	}
    }

    start = 0;
    end = 0;

    MPI_Barrier(MPI_COMM_WORLD);

    /*Only rank 0 read all paths of jobs and run heu 2*/
    if (rank == 0)
    {
	std::vector<struct path*> paths;
	paths.clear();

	for (int i = 0; i < jobs.size(); i++)
	{
	    sprintf(pairfile, "%s_%d", jobfile, jobs[i].job_id);
	    optiq_jobs_read_from_file (jobs, paths, pairfile);
	}

	int unit = 64*1024;
	paths.clear();

	optiq_job_assign_flow_value (jobs, paths, size, unit, 0);

        optiq_job_write_to_file (jobs, jobfile);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /*Every one can read data in now*/

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
