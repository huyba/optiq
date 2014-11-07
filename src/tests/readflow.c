#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include <flow.h>

struct assign_throughput {
    int dest;
    int throughput;
    int flow_id;
    int job_id;
};

#define BASE_UNIT_SIZE 1024

struct optiq_message {
    char *buffer;
    int dest;
    int service_level;
    int length;
    int current_offset;
    int job_id;
    int flow_id;
};

struct optiq_virtual_lane {
    int id;
    queue<struct optiq_message> requests;
};

struct optiq_arbitration {
    int virtual_lane_id;
    int weight;
    int priority;
};

int main(int argc, char **argv)
{
    int world_rank, world_size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    char *file_path = "flow85";

    int num_jobs = 85;
    struct optiq_job *jobs = NULL;
    read_flow_from_file(file_path, &jobs, num_jobs);

    printf("num_jobs = %d\n", num_jobs);

    queue<struct assign_throughput> ab;
    struct optiq_flow *flow = NULL;
    world_rank = 17;
    int *sl2vl;

    for (int i = 0; i < num_jobs; i++) {
	printf("\njob_id = %d, source = %d , dest = %d, num_flows = %d\n", jobs[i].id, jobs[i].source, jobs[i].dest, jobs[i].num_flows);

	for (int j = 0; j < jobs[i].num_flows; j++) {
	    flow = jobs[i].flows.front();
	    jobs[i].flows.pop();

	    printf("flow_id = %d, throughput = %d, num_arcs = %d\n", flow->id, flow->throughput, flow->num_arcs);
	    for (int k = flow->num_arcs-1; k >= 0; k--) {
		printf("%d -> ", flow->arcs[k].ep1);

		if (flow->arcs[k].ep1 == world_rank) {
		    struct assign_throughput b;
		    b.dest = flow->arcs[k].ep2;
		    b.throughput = flow->throughput;
		    b.job_id = jobs[i].id;
		    b.flow_id = flow->id;
		    ab.push(b);
		}
	    }
	    printf("%d\n", flow->arcs[0].ep2);
	}
    }

    while (!ab.empty()) {
	struct assign_throughput b = ab.front();
	ab.pop();
	printf("Node %d will give %d throughput to flow %d of job %d to dest %d\n", world_rank, b.throughput, b.flow_id, b.job_id, b.dest);
    }

    queue<struct optiq_message> messages;

    int buf_size = 2*1024*1024;
    char *buffer = (char*)malloc(buf_size);

    while(!requests.empty()) {
	struct optiq_request req = requests.front();
	requests.pop();

	
    } 

    get_data_size(message, sl2vl);
    optiq_send(buffer, length, dest, flow_id);

    int num_entries_arb_table = 10;
    struct optiq_arbitration *arbitration_table = (struct optiq_arbitration *)malloc(sizeof(struct optiq_arbitration) * num_entries_arb_table);
    
    int num_virtual_lanes = 10;
    struct virtual_lane *virtual_lanes = (struct virtual_lane *)malloc(sizeof(struct virtual_lane)) * num_virtual_lanes);

    /*Go though the arbitration table to get the next virtual lane*/
    bool done = false;
    index = 0;
    while(!done) {
	done = true;
	index = index % num_entries_arb_table;
	index++;

	int virtual_lane_id = arbitration_table[index].virtual_lane_id;
	int nbytes = 0;

	/*Get the virtual lane with that id*/
	for (int i = 0; i < num_virtual_lanes; i++) {
	    if (virtual_lanes[i].id == virtual_lane_id) {
		if(virtual_lanes[i].requests.size() > 0) {
		    struct optiq_message message = virtual_lanes[i].requests.front();

		    nbytes = arbitration_table[i].weight * BASE_UNIT_SIZE;

		    if (message.current_offset + nbytes >= message.length) {
			nbytes = message.length - message.current_offset;
			virtual_lanes[i].requests.pop();
		    }

		    optiq_send((void *)&message.buffer[message.current_offset], nbytes, message.dest, message.flow_id);
		    done = false;

		    /*After process any queue, go back to the arbitration table*/
		    break; 
		}
	    }
	}
    }

    return 0;
}
