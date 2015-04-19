#include <sys/time.h>

#include <mpi.h>

#include "algorithm.h"
#include "model.h"

struct optiq_algorithm *algorithm = NULL;

void optiq_algorithm_init()
{
    if (algorithm == NULL)
    {
	algorithm = (struct optiq_algorithm *) calloc (1, sizeof(struct optiq_algorithm));

	algorithm->search_alg = OPTIQ_ALG_HOPS_CONSTRAINT;
	algorithm->max_hops = 0;

	algorithm->num_paths_per_pair = 1;

	optiq_multibfs_init();

	algorithm->bfs = optiq_multibfs_get();
    }
}

struct optiq_algorithm* optiq_algorithm_get()
{
    if (algorithm == NULL) {
        optiq_algorithm_init();
    }

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

void optiq_algorithm_search_path (std::vector<struct path *> &paths, std::vector<struct job> &jobs, struct multibfs *bfs, int world_rank)
{
    paths.clear();

    std::vector<std::pair<int, std::vector<int> > > source_dests;
    source_dests.clear();

    optiq_job_map_jobs_to_source_dests (jobs, source_dests);

    std::vector<std::pair<int, int> > sd;
    sd.clear();
    for (int i = 0; i < jobs.size(); i++)
    {
	std::pair<int, int> p = std::make_pair (jobs[i].source_id, jobs[i].dest_id);
        sd.push_back(p);
    }

    struct optiq_algorithm *algorithm = optiq_algorithm_get();
    struct optiq_topology *topo = optiq_topology_get();

    if (algorithm->search_alg == OPTIQ_ALG_HOPS_CONSTRAINT) 
    {
	max_path_length = algorithm->max_hops;
	optiq_alg_heuristic_search_manytomany_late_adding_load (paths, source_dests, bfs);
    }

    if (algorithm->search_alg == OPTIQ_ALG_HOPS_CONSTRAINT_EARLY) 
    {
        max_path_length = algorithm->max_hops;
        optiq_alg_heuristic_search_manytomany_early_adding_load (paths, source_dests, bfs);
    }

    if (algorithm->search_alg == OPTIQ_ALG_MPIPLUS)
    {
        optiq_alg_heuristic_search_mpiplus (paths, sd);
    }

    if (algorithm->search_alg == OPTIQ_ALG_KPATHS) 
    {
	char graphFilePath[] = "graph";

	if (world_rank == 0) {
	    optiq_topology_write_graph(topo, 1, graphFilePath);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	optiq_alg_yen_k_shortest_paths (paths, jobs, algorithm->num_paths_per_pair, graphFilePath);
    }

    if (algorithm->search_alg == OPTIQ_ALG_MODEL_PATH_BASED)
    {
	char graphFilePath[] = "graph";

        if (world_rank == 0) {
            optiq_topology_write_graph (topo, 1, graphFilePath);
        }

        MPI_Barrier (MPI_COMM_WORLD);

        optiq_alg_yen_k_shortest_paths (paths, jobs, algorithm->num_paths_per_pair, graphFilePath);

	char jobFile[] = "model.dat";

	optiq_model_write_path_based_model_data (jobFile, jobs, topo->num_nodes, topo->neighbors);

	/* Wait for an solver to solve the problem and read back a solution */
	MPI_Barrier (MPI_COMM_WORLD);

	char pathfile[] = "paths";
        bool available = optiq_model_read_flow_value_from_file (jobFile, jobs);

        if (world_rank == 0) {
            printf("available = %s\n", available ? "true" : "false");
        }

	if (!available) 
        {
	    sleep (10);
            available = optiq_model_read_flow_value_from_file (jobFile, jobs);

            if (world_rank == 0) {
                printf("available = %s\n", available ? "true" : "false");
            }
	}

	MPI_Barrier (MPI_COMM_WORLD);
    }

    for (int i = 0; i < paths.size(); i++)
    {
	if (paths[i]->arcs.front().u == paths[i]->arcs.back().v && paths[i]->arcs.size() > 1) 
	{
	    paths[i]->arcs.clear();
	    struct arc a;
	    a.u = paths[i]->source_id;
	    a.v = paths[i]->source_id;
	    paths[i]->arcs.push_back(a);
	}
    }
}
