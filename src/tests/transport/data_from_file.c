#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "../../core/utils/test.h"
#include "optiq.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct optiq_transport transport;
    optiq_transport_init(&transport, PAMI);

    if (world_rank == 0) {
	printf("Init transport successfully!\n");
    }

    MPI_Barrier(MPI_COMM_WORLD);

    string file_path = "flow85";

    vector<struct optiq_job> jobs;
    read_flow_from_file((char *)file_path.c_str(), jobs);

    struct optiq_vlab vlab;

    optiq_vlab_create(vlab, jobs, world_rank);

    optiq_transport_assign_jobs(&transport, jobs);
    optiq_transport_assign_vlab(&transport, &vlab);

    int data_size = 8*1024*1024;
    char *buffer = (char *)malloc(data_size);

    struct optiq_job local_job;
    for (int i = 0; i < jobs.size(); i++) {
	if (jobs[i].source == world_rank) {
	    local_job = jobs[i];
	}
    }

    /*Adding local job*/
    if (world_rank < 85) {
	local_job.buffer = buffer;
	local_job.demand = data_size;
    }

    int num_iters = 30;
    if (argc > 1) {
	num_iters = atoi(argv[1]);
    }

    struct optiq_message *message = get_message_with_buffer(data_size);

    MPI_Barrier(MPI_COMM_WORLD);
    ring_warm_up(50);

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int iter = 0; iter < num_iters; iter++) {

	if (world_rank < 85) {
	    optiq_vlab_add_job(vlab, local_job, &transport);

	    optiq_vlab_transport(vlab, &transport);

	    bool isDone = false;
	    while (!isDone) {
		isDone = optiq_transport_test(&transport, &local_job);
	    }
	    /*printf("Rank %d done sending data from its job\n", world_rank);*/
	}

	if ( 171 <= world_rank && world_rank <= 255) {
	    message->recv_length = 0;
	    message->header.job_id = world_rank - 171;
	    int isDone = 0;
	    while (isDone == 0) {
		isDone = optiq_transport_recv(&transport, message);
	    }
	    /*printf("Rank %d done receiving data of its job\n", world_rank);*/
	}

	bool done_forward = false;
	while (!done_forward) {
	    done_forward = optiq_pami_transport_forward_test(&transport);
	}
    }

    uint64_t end = GetTimeBase();

    double elapsed_time = (double)(end-start)/1.6e3;
    double max_time;
    MPI_Reduce(&elapsed_time, &max_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (world_rank == 0) {
	double bw = (double) data_size * 1e6 / 1024 / 1024 / max_time / num_iters;
	printf("Elapse time = %8.0f, bw = %8.4f\n", max_time, bw);
    }

    /*printf("Rank %d completed test successfully\n", world_rank);*/

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
