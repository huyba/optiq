#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "manytomany.h"
#include "topology.h"
#include "pami_transport.h"
#include "cesm.h"

using namespace std;

void gather_print_time(uint64_t start, uint64_t end, int iters, long int nbytes, int world_rank)
{
    double elapsed_time = (double)(end - start)/1.6e3;
    double max_time = 0.0;

    MPI_Reduce (&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0)
    {
	max_time = max_time / iters;
	double bw = (double) nbytes / max_time / 1024 / 1024 * 1e6;
	printf("total_data = %ld (MB) t = %8.4f, bw = %8.4f\n", nbytes/1024/1024, max_time, bw);
    }
}

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct multibfs bfs;
    optiq_multibfs_init(bfs);

    if (world_rank == 0) {
	printf("Topology: \n");
	for (int i = 0; i < bfs.num_dims; i++) {
	    printf("%d ", bfs.size[i]);
	}
	printf("\n");
	printf("num_dims = %d, num_nodes = %d\n", bfs.num_dims, bfs.num_nodes);
    }

    vector<int> cpl, land, ice, ocn, atm;

    std::vector<std::pair<int, std::vector<int> > > ocn_cpl, cpl_ocn;

    optiq_cesm_gen(cpl, land, ice, ocn, atm, bfs.num_nodes);
   
    optiq_cesm_gen_couple(cpl, ocn, cpl_ocn);

    int count = 1 * 1024 * 1024;

    if (argc > 1) {
	count = atoi(argv[1]) * 1024;
    }

    std::vector<int> source_ranks = cpl;
    std::vector<int> dest_ranks = ocn;
    std::vector<std::pair<int, std::vector<int> > > source_dests = cpl_ocn;

    int num_dests = dest_ranks.size();
    int num_sources = source_ranks.size();

    int send_bytes = 0;
    char *send_buf;
    int *sendcounts = (int *)calloc(1, sizeof(int) * world_size);
    int *sdispls = (int *)calloc(1, sizeof(int) * world_size);

    for (int i = 0; i < num_sources; i++)
    {
	if (world_rank == source_ranks[i])
	{
	    send_bytes = count * num_dests;
	    send_buf = (char *) malloc(send_bytes);

	    for (int j = 0; j < send_bytes; j++) {
		send_buf[j] = j % 128;
	    }

	    for (int j = 0; j < num_dests; j++)
	    {
		sendcounts[dest_ranks[j]] = count;
		sdispls[dest_ranks[j]] = j * count;
	    }
	}
    }

    int recv_bytes = 0;
    char *recv_buf = NULL;
    int *rdispls = (int *) malloc(sizeof(int) * world_size);
    int *recvcounts = (int *) calloc(1, sizeof(int) * world_size);

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    recv_bytes = num_sources * count;
	    recv_buf = (char *) malloc(recv_bytes);

	    for (int j = 0; j < num_sources; j++) 
	    {
		recvcounts[source_ranks[j]] = count;
		rdispls[source_ranks[j]] = j * count;
	    }
	}
    }

    uint64_t t0 = GetTimeBase();

    max_path_length = bfs.diameter/2;

    std::vector<struct path *> complete_paths;
    complete_paths.clear();

    optiq_alg_heuristic_search_manytomany(complete_paths, source_dests, &bfs);

    /*if (world_rank == 0) {
      optiq_path_print_paths(complete_paths);
    }*/

    MPI_Barrier(MPI_COMM_WORLD);

    struct optiq_schedule schedule;
    schedule.world_rank = world_rank;
    schedule.world_size = world_size;

    schedule.send_buf = send_buf;
    schedule.sendcounts = sendcounts;
    schedule.sdispls = sdispls;

    schedule.recv_buf = recv_buf;
    schedule.recvcounts = recvcounts;
    schedule.rdispls = rdispls;

    schedule.remaining_jobs = num_dests;
    schedule.expecting_length = recv_bytes;
    schedule.sent_bytes = 0;
    schedule.chunk_size = 64 * 1024;

    optiq_schedule_init(schedule);

    /*Create pami_transport and related variables: rput_cookies, message_headers*/
    struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)calloc(1, sizeof(struct optiq_pami_transport));

    pami_transport->sched = &schedule;

    pami_transport->sched->pami_transport = pami_transport;

    optiq_pami_init_extra(pami_transport);
    optiq_pami_init(pami_transport);

    uint64_t t1 = GetTimeBase();

    optiq_schedule_create (schedule, complete_paths);

    //optiq_schedule_print_jobs(schedule);

    for (int nbytes = count; nbytes <= count; nbytes *= 2)
    {
	for (int i = 0; i < schedule.local_jobs.size(); i++)
	{
	    schedule.local_jobs[i].buf_length = nbytes;
	}

	if (world_rank == 0) {
	    printf("\nnbytes = %d\n", nbytes);
        }

	for (int chunk_size = nbytes; chunk_size <= nbytes; chunk_size *=2)
	{
	    if (world_rank == 0) {
		printf("chunk_size = %d\n", chunk_size);
	    }

	    schedule.chunk_size = chunk_size;
	    optiq_schedule_split_jobs (pami_transport, schedule.local_jobs, schedule.chunk_size);

	    /*if (pami_transport->extra.send_headers.size() > 0)
	    {
		printf("Rank %d send_header size = %d\n", world_rank, pami_transport->extra.send_headers.size());
		for (int h = 0; h < pami_transport->extra.send_headers.size() ;h++)
		{
		    struct optiq_message_header *hd = pami_transport->extra.send_headers[h];
		    printf("Rank %d Message size %d from %d to %d follow path_id %d\n", world_rank, hd->length, hd->source, hd->dest, hd->path_id);
		}
	    }*/

	    schedule.remaining_jobs = num_dests;
	    schedule.expecting_length = nbytes * num_sources;
	    schedule.sent_bytes = 0;
	    memset (schedule.recv_bytes, 0, sizeof (int) * world_size);

	    MPI_Barrier (MPI_COMM_WORLD);

	    uint64_t t2 = GetTimeBase();

	    optiq_execute_jobs (pami_transport);

	    uint64_t t3 = GetTimeBase();

	    MPI_Barrier(MPI_COMM_WORLD);

	    double max_t, t = (double)(t3 - t2)/1.6e3;

	    long int data_size = (long int) num_dests * num_sources * nbytes;
	    gather_print_time(t2, t3, 1, data_size, world_rank);
	}
    }

    for (int i = 0; i < num_dests; i++) 
    {
	if (world_rank == dest_ranks[i]) 
	{
	    char *test_buf = (char *) malloc (recv_bytes);

	    for (int i = 0; i < recv_bytes; i++) {
		test_buf[i] = i%128;
	    }

	    if (memcmp (test_buf, recv_buf, recv_bytes) != 0) {
		printf ("Rank %d Received invalid data\n", world_rank);
	    }
	    memset(recv_buf, 0, recv_bytes);
	}
    }

    if (world_rank == 0)
    {
	optiq_path_print_stat(complete_paths, bfs.num_nodes);

	printf("\nTest with MPI_Alltoallv\n");
    }

    int iters = 30;

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int i = 0; i < iters; i++)
    {
	MPI_Alltoallv(send_buf, sendcounts, sdispls, MPI_BYTE, recv_buf, recvcounts, rdispls, MPI_BYTE, MPI_COMM_WORLD);
    }

    uint64_t end = GetTimeBase();

    long int data_size = (long int) num_dests * num_sources * count;
    gather_print_time(start, end, iters, data_size, world_rank);

    for (int i = 0; i < num_dests; i++)
    {
        if (world_rank == dest_ranks[i])
        {
            char *test_buf = (char *) malloc (recv_bytes);

            for (int i = 0; i < recv_bytes; i++) {
                test_buf[i] = i%128;
            }

            if (memcmp (test_buf, recv_buf, recv_bytes) != 0) {
                printf ("Rank %d Received invalid data\n", world_rank);
            }
        }
    }

    MPI_Finalize();

    return 0;
}
