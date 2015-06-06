#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

void gen_91_cases (struct optiq_topology *topo, int demand, char *graphFilePath, int numpaths);

void gen_cesm_paths();

void read_cesm_patterns_gen_paths();
