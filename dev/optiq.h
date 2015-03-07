#ifndef OPTIQ_H
#define OPTIQ_H

#include "util.h"
#include "opi.h"

#include "topology.h"
#include "pathreconstruct.h"

#include "algorithm.h"

#include "path.h"
#include "job.h"
#include "heap_path.h"

#include "schedule.h"
#include "comm_mem.h"

#include "pami_transport.h"

#include "cesm.h"
#include "patterns.h"

/*
 * Init the optiq lib. Will init topology, algorithm, schedule and transport.
 * */
void optiq_init(int argc, char **argv);

/*
 * Print basic information.
 * */
void optiq_print_basic ();

/*
 * Finalize optiq lib. Will finalize topology, algorithm, schedule and transport.
 * */
void optiq_finalize();

/*
 *  Transport data in MPI_Alltoallv style.
 * */
void optiq_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls);

#endif
