#include "yen.h"
#include "topology.h"
#include "job.h"
#include <mpi.h>
#include <vector>

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    optiq_topology_init();

    std::vector<struct path *> complete_paths;
    std::vector<struct job> jobs;
    int num_paths = 3;
    char graphFilePath[] = "graph";

    int id = 0;
    for (int i = 0; i < size/2; i++) 
    {
	struct job new_job;
	new_job.source_id = i/8;
	new_job.dest_id = (i+size/2)/8;
	new_job.job_id = id;
	id++;

	jobs.push_back(new_job);
    }
    
    if (rank == 0) {
	optiq_topology_write_graph (topo, 1, graphFilePath);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    optiq_alg_yen_k_shortest_paths(complete_paths, jobs, num_paths, graphFilePath);

    if (rank == 0) {
	optiq_path_print_paths(complete_paths);
    }
}
