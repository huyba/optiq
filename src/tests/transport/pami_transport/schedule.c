#include "schedule.h"

#define OPTIQ_MAX_NUM_PATHS (1024 * 1024)

void optiq_schedule_init(struct optiq_schedule &schedule)
{
    schedule.next_dests = (int *) calloc (1, sizeof(int) * OPTIQ_MAX_NUM_PATHS);
    schedule.final_dest = (int *) calloc (1, sizeof(int) * schedule.world_size);
    schedule.flow_id = (int *) calloc (1, sizeof(int) * schedule.world_size);
    schedule.recv_bytes = (int *) calloc (1, sizeof(int) * world_size);
}

void build_next_dests(int world_rank, int *next_dests, std::vector<struct path *> &complete_paths)
{
    for (int i = 0; i < complete_paths.size(); i++)
    {
        for (int j = 0; j < complete_paths[i]->arcs.size(); j++)
        {
            if (complete_paths[i]->arcs[j].u == world_rank)
            {
                next_dests[i] = complete_paths[i]->arcs[j].v;
            }
        }
    }
}

void optiq_schedule_create (struct optiq_schedule &schedule, std::vector<struct path *> &complete_paths)
{
    struct optiq_pami_transport *pami_transport = schedule.pami_transport;

    memset(schedule.next_dests, 0, sizeof(int) * OPTIQ_MAX_NUM_PATHS);

    build_next_dests(schedule.world_rank, schedule.next_dests, complete_paths);

    int *final_dest = schedule.final_dest;
    int *flow_id = schedule.flow_id;

    int world_rank = schedule.world_rank;

    bool isSource = false, isDest = false;

    int index = 0;
    for (int i = 0; i < complete_paths.size(); i++) {
        if (complete_paths[i]->arcs.back().v == world_rank) {
            isDest = true;
        }

        if (complete_paths[i]->arcs.front().u == world_rank) {
            isSource = true;
            flow_id[index] = i;
            final_dest[index] = complete_paths[i]->arcs.back().v;
            index++;
        }
    }

    schedule.isDest = isDest;

    schedule.send_mr.offset = 0;
    scheudle.recv_mr.offset = 0;

    size_t bytes;
    pami_result_t result;

    if (isSource)
    {
        result = PAMI_Memregion_create (pami_transport->context, schedule.send_buf, schedule.send_bytes, &bytes, &schedule.send_mr.mr);

        if (result != PAMI_SUCCESS) {
            printf("No success\n");
        } else if (bytes < schedule.send_bytes) {
            printf("Registered less\n");
        }
    }

    if (isDest)
    {
        result = PAMI_Memregion_create (pami_transport->context, schedule.recv_buf, schedule.recv_bytes, &bytes, &schedule.recv_mr.mr);

        if (result != PAMI_SUCCESS) {
            printf("No success\n");
        } else if (bytes < schedule.recv_bytes) {
            printf("Registered less\n");
        }
    }

    int nbytes = 32 * 1024;
    if (isSource) {
	for (int i = 0; i < schedule.world_size; i++)
	{
	    if (schedule.sendcounts[i] != 0) {
		
	    }
	}


        for (int offset = 0; offset < schedule.send_bytes; offset += nbytes) {
            for (int i = 0; i < num_dests; i++) {
                struct optiq_message_header *header = pami_transport->extra.message_headers.back();
                pami_transport->extra.message_headers.pop_back();

                header->length = nbytes;
                header->source = world_rank;
                header->dest = final_dest[i];
                header->path_id = flow_id[i];

                memcpy(&header->mem, &bulk->send_mr, sizeof(struct optiq_memregion));
                header->mem.offset = offset;
                header->original_offset = offset;

                pami_transport->extra.send_headers.push_back(header);
            }
        }
    }
}
