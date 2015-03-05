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

void optiq_init(int argc, char **argv);

void optiq_print_basic ();

void optiq_finalize();

void optiq_alltoallv(void *sendbuf, int *sendcounts, int *sdispls, void *recvbuf, int *recvcounts, int *rdispls);

#endif
