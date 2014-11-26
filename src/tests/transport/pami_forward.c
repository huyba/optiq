#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "optiq.h"

#define OPTIQ_DEBUG_TRANSPORT

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct optiq_transport transport;
    optiq_transport_init(&transport, PAMI);

#ifdef DEBUG
    if (world_rank == 0) {
	printf("Debuging mode\n");
    }
#endif

    if (world_rank == 0) {
	printf("Init transport successfully!\n");
    }

    int data_size = 2 * 1024 * 1024;
    if (argc > 1) {
	data_size  = atoi(argv[1]) * 1024;
    }

    if (world_rank == 0) {
	printf("Data size is %d\n", data_size);
    }

    char *buffer = (char*)malloc(data_size);
    for (int i = 0; i < data_size; i++) {
	buffer[i] = i % 128;
    }

    /*Create jobs for testing*/
    /*2 jobs here: 0-1-2 and 1-2-3*/
    struct optiq_flow flow0, flow1;
    struct optiq_arc arc0, arc1, arc2;
    arc0.ep1 = 0;
    arc0.ep2 = 1;
    arc1.ep1 = 1;
    arc1.ep2 = 2;
    arc2.ep1 = 2;   
    arc2.ep2 = 3;

    flow0.id = 0;
    flow0.throughput = 1024;
    flow0.arcs.push_back(arc0);
    flow0.arcs.push_back(arc1);

    flow1.id = 1;
    flow1.throughput = 1024;
    flow1.arcs.push_back(arc1);
    flow1.arcs.push_back(arc2);

    struct optiq_job job0, job1;

    job0.id = 0;
    job0.source = 0;
    job0.dest = 2;
    job0.demand = data_size;
    job0.buffer = buffer;
    job0.flows.push_back(flow0);

    job1.id = 1;
    job1.source = 1;
    job1.dest = 3;
    job1.demand = data_size;
    job1.buffer = buffer;
    job1.flows.push_back(flow1);

    MPI_Barrier(MPI_COMM_WORLD);
    vector<struct optiq_job> jobs;
    jobs.push_back(job0);
    jobs.push_back(job1);

    struct optiq_vlab vlab;

    optiq_vlab_create(vlab, jobs, world_rank);
    optiq_transport_assign_jobs(&transport, jobs);
    optiq_transport_assign_vlab(&transport, &vlab);

    MPI_Barrier(MPI_COMM_WORLD);

    /*Iterate the arbitration table to get the next virtual lane*/
    int num_iters = 1;
    if (argc > 2) {
	num_iters = atoi(argv[2]);
    }
    struct optiq_message *message = NULL;
    for (int iter = 0; iter < num_iters; iter++)
    {
	if (world_rank <= 1) {
	    optiq_vlab_add_job(vlab, jobs[world_rank], &transport);

	    optiq_vlab_transport(vlab, &transport);

	    bool isDone = false;
	    while (!isDone) {
		isDone = optiq_transport_test(&transport, &jobs[world_rank]);
	    }
	}

	if (2 <=  world_rank && world_rank <= 3) {
	    message = get_message_with_buffer(data_size);
	    message->header.job_id = world_rank - 2;
	    int isDone = 0;
	    while (isDone == 0) {
		isDone = optiq_transport_recv(&transport, message);
	    }
	}

	bool done_forward = false;
	while (!done_forward) {
	    done_forward = optiq_pami_transport_forward_test(&transport);
	}
    }

    if (2 <= world_rank && world_rank <= 3) {
	if (memcmp(message->buffer, buffer, data_size) != 0) {
	    printf("Error: Rank %d received invalid data\n", world_rank);
	    for (int i = 0; i < data_size; i++) {
		if (message->buffer[i] != buffer[i]) {
		    printf("Rank %d Pos %d: recv: %d sent: %d\n", world_rank, i, message->buffer[i], buffer[i]);
		}
	    }
	    printf("\n");
	} else {
	    printf("Rank %d received valid data\n", world_rank);
	}
    }

    printf("Rank %d completed the test successfully\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
