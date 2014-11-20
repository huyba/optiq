#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "job.h"
#include "flow.h"
#include "message.h"
#include "virtual_lane.h"
#include "transport.h"

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

    int data_size = 2*1024*1024;
    char *buffer = (char*)malloc(data_size);

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

    vector<struct optiq_arbitration> arbitration_table;
    vector<struct optiq_virtual_lane> virtual_lanes;

    create_virtual_lane_arbitration_table(virtual_lanes, arbitration_table, jobs, world_rank);

    optiq_transport_assign_jobs(&transport, &jobs);
    optiq_transport_assign_virtual_lanes(&transport, &virtual_lanes, &arbitration_table);

    MPI_Barrier(MPI_COMM_WORLD);

    /*Iterate the arbitration table to get the next virtual lane*/
    for (int iter = 0; iter < 5; iter++)
    {
	if (world_rank <= 1) {
	    add_job_to_virtual_lanes(jobs[world_rank], &virtual_lanes);

	    transport_from_virtual_lanes(&transport, virtual_lanes, arbitration_table);

	    bool isDone = false;
	    while (!isDone) {
		isDone = optiq_transport_test(&transport, &jobs[world_rank]);
	    }
	    printf("Rank %d done sending data from its job\n", world_rank);
	}

	if (2 <=  world_rank && world_rank <= 3) {
	    struct optiq_message *message = get_message_with_buffer(data_size);
	    message->header.job_id = world_rank - 2;
	    int isDone = 0;
	    while (isDone == 0) {
		isDone = optiq_transport_recv(&transport, message);
	    }
	    printf("Rank %d done receiving data of its job\n", world_rank);
	}

	bool done_forward = false;
	while (!done_forward) {
	    done_forward = optiq_pami_transport_forward_test(&transport);
	}
    }

    printf("Rank %d completed the test successfully\n", world_rank);

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
