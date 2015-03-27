#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <mpi.h>

#include "optiq.h"
#include "optiq_benchmark.h"

int main(int argc, char **argv)
{
    optiq_init (argc, argv);

    struct topology *topo = optiq_topology_get();
    struct multibfs *bfs = optiq_multibfs_get();
    struct optiq_algorithm *algorithm = optiq_algorithm_get();
    struct optiq_pami_transport *pami_transport = optiq_pami_transport_get();
    struct optiq_schedule *sched = optiq_schedule_get();

    algorithm->search_alg = OPTIQ_ALG_MPIPLUS;

    int rank = pami_transport->rank;
    int size = pami_transport->size;
    int n = size;

    int demand = 4 * 1024 * 1024;
    int testid = 1;
    sched->auto_chunksize = false;
    sched->chunk_size = 64 * 1024;

    if (rank == 0) {
	printf("Dijoint testings\n");
    }
    char *filepath = "pattern";


    /*if (rank == 0)
    {
	printf ("Test %d: %d ranks from %d to %d send data to %d rank from %d to %d\n", testid, 64, 0, 64-1, 64, 64*6, 64*7-1);
	optiq_pattern_m_to_n (filepath, size, demand, 64, 0, 64, 64*6, false);
	testid++;
    }

    MPI_Barrier(MPI_COMM_WORLD);
for (int chunk = 8 * 1024; chunk < 128 * 1024; chunk *= 2) {
	sched->auto_chunksize = false;
	sched->chunk_size = chunk;
	optiq_benchmark_pattern_from_file (filepath, rank, size);
    }*/

    for (int i = 2; i <= 8; i *= 2)
    {
	for (int j = 1; j < i; j++) 
	{
	    for (int r = 1; r <= 4; r *= 2)
	    {
		if (rank == 0) 
		{
		    printf ("Test %d: %d ranks from %d to %d send data to %d rank from %d to %d\n", testid, n/i, 0, n/i-1, n/i/r, j*n/i, n/i/r + j*n/i - 1);
		    optiq_pattern_m_to_n (filepath, size, demand, n/i, 0, n/i/r, j*n/i, false);
		    testid++;
		}

		MPI_Barrier(MPI_COMM_WORLD);
		optiq_benchmark_pattern_from_file (filepath, rank, size);
	    }
	}
    }

    if (rank == 0) {
	printf("Overlap testings\n");
    }
    for(int i = 2; i <= 8; i *= 2)
    {
	for (int j = 0; j < i - 1; j++)
	{
	    for (int k = 2; k <= 4; k *= 2)
	    {
		for (int r = 1; r <= 4; r *= 2)
		{
		    if (rank == 0) 
		    {
			printf ("Test %d: %d ranks from %d to %d send data to %d rank from %d to %d\n", testid, n/i, j*n/i, n/i + j*n/i - 1, n/i/r , j*n/i + n/i - n/i/r/k, n/i/r + j*n/i + n/i - n/i/r/k - 1);
			optiq_pattern_m_to_n (filepath, size, demand, n/i, j*n/i, n/i/r, j*n/i + n/i - n/i/r/k, false);
			testid++;
		    }

		    MPI_Barrier(MPI_COMM_WORLD);
		    optiq_benchmark_pattern_from_file (filepath, rank, size);

		}
	    }
	}
    }

    if (rank == 0) {
	printf("Subset testings\n");
    }

    for(int i = 2; i <= 8; i *= 2)
    {
	for (int j = 0; j < i; j++)
	{
	    for (int k = 2; k <= 4; k *= 2)
	    {
		for (int r = 2; r <= 4; r *= 2)
		{

		    if (rank == 0) 
		    {
			printf ("Test %d: %d ranks from %d to %d send data to %d rank from %d to %d\n", testid, n/i, j*n/i, n/i + j*n/i - 1, n/i/r, j*n/i + n/i/r/k, n/i/r + j*n/i + n/i/r/k - 1);
			optiq_pattern_m_to_n (filepath, size, demand, n/i, j*n/i, n/i/r, j*n/i + n/i/r/k, false);
			testid++;
		    }

		    MPI_Barrier(MPI_COMM_WORLD);
		    optiq_benchmark_pattern_from_file (filepath, rank, size);
		}
	    }
	}
    }

    return 0;
}

