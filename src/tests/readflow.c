#include <stdlib.h>
#include <stdio.h>

#include <flow.h>

int main(int argc, char **argv)
{
    char *file_path = "flow85";

    int num_jobs = 85;
    struct optiq_job *jobs = NULL;
    read_flow_from_file(file_path, jobs, num_jobs);

    printf("num_jobs = %d\n", num_jobs);

    struct optiq_flow *flow = NULL;
    for (int i = 0; i < num_jobs; i++) {
	printf("\njob_id = %d, source = %d , dest = %d, num_flows = %d\n", jobs[i].id, jobs[i].source, jobs[i].dest, jobs[i].num_flows);

	for (int j = 0; j < jobs[i].num_flows; j++) {
	    flow = jobs[i].flows.front();
	    jobs[i].flows.pop();

	    printf("flow_id = %d, throughput = %d, num_arcs = %d\n", flow->id, flow->throughput, flow->num_arcs);
	    for (int k = flow->num_arcs-1; k >= 0; k--) {
		printf("%d -> ", flow->arcs[k].ep1);
	    }
	    printf("%d\n", flow->arcs[0].ep2);
	}
    }

    return 0;
}
