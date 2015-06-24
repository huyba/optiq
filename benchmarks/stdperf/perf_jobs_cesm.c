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
        mindemand = atoi(argv[4]);
    }

    if (argc > 5) {
        demand = atoi (argv[5]);
    }

    int minchunksize = 64 * 1024;
    int maxchunksize = 128 * 1024;

    if (argc > 6) {
        minchunksize = atoi (argv[6]) * 1024;
    }

    if (argc > 7) {
        maxchunksize = atoi (argv[7]) * 1024;
    }

    char jobfile[256];

    if (rank == 0) {
        printf("MPI/OPTIQ msg chunk totalsize time(us) bandwidth(MB/s) #path totalhops totalloads loadlinks maxhops minhops avghops medhops maxload minload avgload medload\n\n");
    }

    for (int nbytes = demand; nbytes >= mindemand; nbytes /= 2)
    {
        for (int id = start; id <= end; id++)
        {
            schedule->test_id = id;

            sprintf(jobfile, "%s/test%d", path, id);

            if (rank == 0) {
                printf("Test No. %d\n", id);
            }

            /*int minchunk = minchunksize;
              if (nbytes < minchunk) {
              minchunk = nbytes;
              }

              int maxchunk = maxchunksize;
              if (nbytes < maxchunk) {
              maxchunk = nbytes;
              }*/

            for (int chunk = minchunksize; chunk <= maxchunksize; chunk *= 2)
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

                std::vector<struct job> &jobs = schedule->jobs;
                jobs.clear();
                std::vector<struct path *> &path_ids = opi.paths, &path_ranks = schedule->paths;
                path_ranks.clear();

                /* When reading from file, always return jobs with path of node ids, not ranks */
                optiq_jobs_read_from_file (jobs, path_ranks, jobfile);

                MPI_Barrier(MPI_COMM_WORLD);

                if (jobs.size() == 0 || path_ranks.size() == 0)
                {
                    if (rank == 0) {
                        printf("No jobs or paths found from file %s\n", jobfile);
                    }

                    break;
                }

                /* Check if enough paths to send, if not return. This should not happen, but still in current alg. */
                for (int i = 0; i < jobs.size(); i++)
                {
                    if (jobs[i].paths.size() == 0)
                    {
                        for (int j = 0; j < path_ranks.size(); j++)
                        {
                            free (path_ranks[i]);
                        }

                        if (rank == 0)
                        {
                            printf("Not enought paths\n");
                        }

                        break;
                    }
                }

                /* Convert to have paths with node ids and jobs with ranks */
                optiq_jobs_convert_ids_to_ranks (jobs, path_ids, topo->num_ranks_per_node);

                for (int i = 0; i < jobs.size(); i++)
                {
                    jobs[i].demand = jobs[i].demand * nbytes;
                }

                bool ret = optiq_benchmark_jobs (jobs);

                if (!ret) {
                    break;
                }

                opi.iters = 1;
                optiq_opi_collect();

                if (rank == 0) 
                {
                    sprintf(opi.prefix, "%s", "O");
                    opi.test_id = id;
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
