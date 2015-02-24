ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 #-DDEBUG
endif

LD      = mpixlc_r

OPTIQ = ../../..
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
TOPO = $(DEV)/topology
STRUCT = $(DEV)/structures
ALG = $(DEV)/algorithms
YEN = $(ALG)/yen
HEU = $(ALG)/heuristics
TRANS = $(DEV)/transport/pami_transport
PAT = $(DEV)/comm_patterns
SCHED= $(DEV)/schedule

INCLUDE = -I. -I$(UTIL) -I$(TOPO) -I$(STRUCT) -I$(HEU) -I$(YEN) -I$(PAT) -I$(TRANS) -I$(SCHED)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

YEN_OBJ = $(YEN)/yen.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/YenTopKShortestPathsAlg.o

OBJ = $(TRANS)/pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o $(ALG)/heuristics/alltomany.o $(ALG)/heuristics/manytomany.o $(PAT)/cesm.o $(ALG)/heuristics/multibfs.o $(SCHED)/schedule.o $(SCHED)/comm_mem.o $(PAT)/patterns.o $(ALG)/heuristics/multipaths.o

all: $(OBJ) yen mpi copy

yen:
	cd $(YEN)/ && $(MAKE) -f Makefile

mpi: perf_manytomany_cesm.x perf_alltomany.x perf_manytomany_patterns.x perf_kpaths_patterns.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ) $(YEN_OBJ)
	$(LD) $< $(OBJ) $(YEN_OBJ) -o $@ $(LDFLAGS)

clean:
	rm $(OBJ) *.x *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

BENCHMARK_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/benchmarks

copy:
	cp perf_manytomany_cesm.x $(BENCHMARK_DIR)/algorithms/heuristics/manytomany/cesm
	cp perf_manytomany_patterns.x $(BENCHMARK_DIR)/algorithms/heuristics/manytomany/patterns
	cp perf_kpaths_patterns.x $(BENCHMARK_DIR)/algorithms/heuristics/kpaths/patterns
	cp perf_alltomany.x $(BENCHMARK_DIR)/algorithms/heuristics/alltomany
