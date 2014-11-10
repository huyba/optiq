#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <string>

#include <mpi.h>

#include "virtuallane.h"
#include "flow.h"
#include "transport.h"

using namespace std;

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    struct optiq_transport transport;
    optiq_transport_init(&transport, NONBLK_MPI);

    string file_path = "flow85";

    int num_jobs = 85;
    struct optiq_job *jobs = NULL;
    read_flow_from_file((char *)file_path.c_str(), &jobs, num_jobs);

    world_rank = 17;

    vector<struct optiq_arbitration> arbitration_table;
    vector<struct optiq_virtual_lane> virtual_lanes;

    create_virtual_lane_arbitration_table(virtual_lanes, arbitration_table, num_jobs, jobs, world_rank);

    int data_size = 4*1024*1024;
    char *buffer = (char *)malloc(data_size);

    struct optiq_job local_job;
    for (int i = 0; i < num_jobs; i++) {
        if (jobs[i].source == world_rank) {
            local_job = jobs[i];
        }
    }

    add_message_to_virtual_lanes(buffer, data_size, local_job, virtual_lanes);

    /*Iterate the arbitration table to get the next virtual lane*/
    transfer_from_virtual_lanes(&transport, arbitration_table, virtual_lanes);

    return 0;
}
