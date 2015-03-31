#include <sys/time.h>

#include <mpi.h>

#include "algorithm.h"
#include "model.h"

struct optiq_algorithm *algorithm = NULL;

void optiq_algorithm_init()
{
    algorithm = (struct optiq_algorithm *) calloc (1, sizeof(struct optiq_algorithm));

    algorithm->search_alg = OPTIQ_ALG_HOPS_CONSTRAINT;
    algorithm->max_hops = 0;

    algorithm->num_paths_per_pair = 1;

    optiq_multibfs_init();

    algorithm->bfs = optiq_multibfs_get();
}

struct optiq_algorithm* optiq_algorithm_get()
{
    return algorithm;
}

void optiq_algorithm_set_search_algorithm(enum search_algorithm search_alg)
{
    struct optiq_algorithm *algorithm = optiq_algorithm_get();
    algorithm->search_alg = search_alg;
}

void optiq_algorithm_finalize()
{
    free(algorithm);
    optiq_multibfs_finalize();
}

void optiq_algorithm_destroy()
{
    /*if (bfs->paths != NULL) 
    {
	free(bfs->paths);
	bfs->paths = NULL;
    }*/
}

void optiq_algorithm_search_path(std::vector<struct path *> &paths, std::vector<struct job> &jobs, struct multibfs *bfs, int world_rank)
{
    std::vector<std::pair<int, std::vector<int> > > source_dests;
    optiq_job_map_jobs_to_source_dests (jobs, source_dests);

    struct optiq_algorithm *algorithm = optiq_algorithm_get();
    struct topology *topo = optiq_topology_get();
    
    if (algorithm == NULL) {
	optiq_algorithm_init();
    }

    if (algorithm->search_alg == OPTIQ_ALG_HOPS_CONSTRAINT) {
	max_path_length = algorithm->max_hops;
	optiq_alg_heuristic_search_manytomany_late_adding_load (paths, source_dests, bfs);
    }

    if (algorithm->search_alg == OPTIQ_ALG_HOPS_CONSTRAINT_EARLY) 
    {
        max_path_length = algorithm->max_hops;
        optiq_alg_heuristic_search_manytomany_early_adding_load (paths, source_dests, bfs);
    }

    if (algorithm->search_alg == OPTIQ_ALG_KPATHS) 
    {
	char graphFilePath[] = "graph";

	if (world_rank == 0) {
	    optiq_graph_print_graph(bfs, 1, graphFilePath);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	optiq_alg_yen_k_shortest_paths (paths, jobs, algorithm->num_paths_per_pair, graphFilePath);
    }

    if (algorithm->search_alg == OPTIQ_ALG_MPIPLUS)
    {
	optiq_alg_heuristic_search_mpiplus (paths, source_dests);
    }

    if (algorithm->search_alg == OPTIQ_ALG_MODEL_PATH_BASED)
    {
	char graphFilePath[] = "graph";

        if (world_rank == 0) {
            optiq_graph_print_graph(bfs, 1, graphFilePath);
        }

        MPI_Barrier (MPI_COMM_WORLD);

        optiq_alg_yen_k_shortest_paths (paths, jobs, algorithm->num_paths_per_pair, graphFilePath);

	char jobFile[] = "model.dat";

	optiq_model_write_path_based_model_data (jobs, jobFile, topo->num_nodes, topo->neighbors);

	/* Wait for an solver to solve the problem and read back a solution */
	MPI_Barrier (MPI_COMM_WORLD);

	if (!optiq_path_read_flow_value_from_file (jobFile, jobs)) {
	    sleep (10);
	}

	MPI_Barrier (MPI_COMM_WORLD);
    }
}
