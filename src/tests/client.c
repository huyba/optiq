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

    MPI_Barrier(MPI_COMM_WORLD);

    string file_path = "flow85";

    vector<struct optiq_job> jobs;
    read_flow_from_file((char *)file_path.c_str(), jobs);

    vector<struct optiq_arbitration> arbitration_table;
    vector<struct optiq_virtual_lane> virtual_lanes;

    create_virtual_lane_arbitration_table(virtual_lanes, arbitration_table, jobs, world_rank);

    transport.virtual_lanes = &virtual_lanes;
    transport.jobs = &jobs;

    int data_size = 4*1024*1024;
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
        add_job_to_virtual_lanes(local_job, &virtual_lanes);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /*Iterate the arbitration table to get the next virtual lane*/
    if (world_rank < 85) {
        transport_from_virtual_lanes(&transport, arbitration_table, virtual_lanes);

        bool isDone = false;
        while (!isDone) {
            isDone = optiq_transport_test(&transport, &local_job);
        }
    }

    if ( 171 <= world_rank && world_rank <= 255) {
        struct optiq_message *message = get_message_with_buffer(data_size);
        int isDone = 0;
        while (isDone == 0) {
            isDone = optiq_transport_recv(&transport, message);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
