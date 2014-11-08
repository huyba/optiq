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

void optiq_send(void *buffer, int length, int dest, int flow_id)
{

}

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
    struct optiq_job local_job;

    printf("num_jobs = %d\n", num_jobs);

    queue<struct assign_throughput> ab;
    struct optiq_flow *flow = NULL;

    world_rank = 17;
    int total_local_throughput = 0;

    for (int i = 0; i < num_jobs; i++) {
	printf("\njob_id = %d, source = %d , dest = %d, num_flows = %d\n", jobs[i].id, jobs[i].source, jobs[i].dest, jobs[i].num_flows);

	if (jobs[i].source == world_rank) {
	    local_job = jobs[i];
	}

	for (int j = 0; j < jobs[i].num_flows; j++) {
	    flow = jobs[i].flows[j];

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

	    /*Compute the total flows for the local node*/
	    if (jobs[i].source == world_rank) {
		total_local_throughput += flow->throughput;
	    }
	}
    }

    /*Arbitration table of virtual_lane_id and assigned weight*/
    int num_entries_arb_table = ab.size();
    struct optiq_arbitration *arbitration_table = (struct optiq_arbitration *)malloc(sizeof(struct optiq_arbitration) * num_entries_arb_table);   

    /*Virtual lanes*/
    int num_virtual_lanes = ab.size();
    struct optiq_virtual_lane *virtual_lanes = (struct optiq_virtual_lane *)malloc(sizeof(struct optiq_virtual_lane) * num_virtual_lanes);

    /*Fill in the arbitration table*/
    int index = 0;
    while (!ab.empty()) {
	struct assign_throughput b = ab.front();
	ab.pop();
	arbitration_table[index].virtual_lane_id = b.flow_id;
	arbitration_table[index].weight = b.throughput;
	arbitration_table[index].priority = 0;
	virtual_lanes[index].id = b.flow_id;
	printf("Node %d will give %d throughput to flow %d of job %d to dest %d\n", world_rank, b.throughput, b.flow_id, b.job_id, b.dest);
    }

    int data_size = 4*1024*1024;
    char *buffer = (char *)malloc(data_size);

    /*Fill in the virtual lanes with data from local jobs*/
    int global_offset = 0, length = 0;
    for (int i = 0; i < local_job.num_flows; i++) {
	length = (int)((double)local_job.flows[i]->throughput / (double)total_local_throughput) * data_size;
	struct optiq_message message;
	message.job_id = local_job.id;
	message.flow_id = local_job.flows[i]->id;
	message.dest = local_job.dest;
	message.current_offset = 0;
	message.service_level = 0;
	message.buffer = &buffer[global_offset];
	message.length = length;
	global_offset += length;

	for (int j = 0; j < num_virtual_lanes; j++) {
	    if (message.flow_id == virtual_lanes[i].id) {
		virtual_lanes[i].requests.push(message);
	    }
	}
    }


    /*Iterate the arbitration table to get the next virtual lane*/
    bool done = false;
    int nbytes = 0;
    int virtual_lane_id = 0;
    while(!done) {
	done = true;

	/*Get the virtual lane id*/
	index = index % num_entries_arb_table;
	virtual_lane_id = arbitration_table[index].virtual_lane_id;
	index++;

	/*Get the virtual lane with that id*/
	for (int i = 0; i < num_virtual_lanes; i++) {
	    if (virtual_lanes[i].id == virtual_lane_id) {
		if(virtual_lanes[i].requests.size() > 0) {
		    struct optiq_message message = virtual_lanes[i].requests.front();

		    nbytes = arbitration_table[index].weight * BASE_UNIT_SIZE;

		    if (message.current_offset + nbytes >= message.length) {
			nbytes = message.length - message.current_offset;
			virtual_lanes[i].requests.pop();
		    } else {
			/*Update the virtual lane*/
			message.current_offset += nbytes;
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
