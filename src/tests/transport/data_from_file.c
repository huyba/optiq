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

    char *file_path = (char *)"flow85";
    if (argc > 1) {
	file_path = argv[1];
    }

    vector<struct optiq_job> jobs;
    optiq_job_read_from_file(file_path, &jobs);

    /*if (world_rank == 0) {
	optiq_job_print(&jobs);
    }*/

    struct optiq_vlab vlab;

    optiq_vlab_create(vlab, jobs, world_rank);

    optiq_transport_assign_jobs(&transport, jobs);
    optiq_transport_assign_vlab(&transport, &vlab);

    int data_size = 8*1024*1024;
    if (argc > 2) {
	data_size = atoi(argv[2]) * 1024;
    }
    char *buffer = (char *)malloc(data_size);
    for (int i = 0; i < data_size;  i++) {
	buffer[i] = i % 128;
    }

    struct optiq_job local_job;
    bool isSource = false, isDest = false;
    for (int i = 0; i < jobs.size(); i++) {
	if (jobs[i].source == world_rank) {
	    local_job = jobs[i];
	    isSource = true;
	}
	if (jobs[i].dest == world_rank) {
	    local_job = jobs[i];
	    isDest = true;
	}
    }

    /*Adding local job*/
    if (isSource) {
	local_job.buffer = buffer;
	local_job.demand = data_size;
    }

    int num_iters = 30;
    if (argc > 3) {
	num_iters = atoi(argv[3]);
    }

    struct optiq_message *message = get_message_with_buffer(data_size);

    MPI_Barrier(MPI_COMM_WORLD);
    /*ring_warm_up(50);*/

    MPI_Barrier(MPI_COMM_WORLD);

    uint64_t start = GetTimeBase();

    for (int iter = 0; iter < num_iters; iter++) {

	if (isSource) {    
	    optiq_vlab_add_job(vlab, local_job, &transport);

	    if (world_rank == 0) {
		print_virtual_lanes(vlab.vl);
	    }

	    optiq_vlab_transport(vlab, &transport);

	    bool isDone = false;
	    while (!isDone) {
		isDone = optiq_transport_test(&transport, &local_job);
	    }
	    /*printf("Rank %d done sending data from its job\n", world_rank);*/
	}

	if (isDest) {
	    message->recv_length = 0;
	    message->header.job_id = local_job.id;
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

    if (isDest) {
	if (memcmp(message->buffer, buffer, data_size) != 0) {
	    printf("Rank %d: invalid data received\n", world_rank);
	} /*else {
	    printf("Rank %d: valid data received\n", world_rank);
	}*/
    }

    /*printf("Rank %d completed test successfully\n", world_rank);*/

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
