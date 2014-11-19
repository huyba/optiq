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

    int data_size = 4*1024*1024;
    char *buffer = (char *)malloc(data_size);

    MPI_Barrier(MPI_COMM_WORLD);

    struct optiq_message *message = get_message_with_buffer(data_size);
    message->header.flow_id = 0;
    message->header.final_dest = 1;
    message->header.original_length = data_size;
    message->header.original_offset = 0;
    message->header.original_source = 0;
    message->length = data_size;
    

    /*Iterate the arbitration table to get the next virtual lane*/
    if (world_rank == 0) {
	message->next_dest = 1;

	optiq_transport_send(&transport, message);

        bool isDone = false;
	struct optiq_pami_transport *pami_transport = (struct optiq_pami_transport *)optiq_transport_get_concrete_transport(&transport);
        while (!isDone) {
	    PAMI_Context_advance (pami_transport->context, 100);
	    if (pami_transport->in_use_send_cookies.size() > 0) {
		isDone = true;
		pami_transport->in_use_send_cookies.pop_back();
	    }
        }
    }

    if ( world_rank == 1) {
        int isDone = 0;
        while (isDone == 0) {
            isDone = optiq_transport_recv(&transport, message);
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    MPI_Finalize();

    return 0;
}
