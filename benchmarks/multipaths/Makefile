ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 #-DDEBUG
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),gxx)
    CC	    = g++
    COPT    = -g -Wall
    LG	    = g++
endif

OPTIQ = ../..
DEV = $(OPTIQ)/dev
BM = $(OPTIQ)/benchmarks
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
ALG = $(DEV)/algorithms
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV) -I$(BM)/mpi -I$(BM)/optiq/
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(ALG)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o $(DEV)/optiq.o $(BM)/mpi/*.o $(BM)/optiq/optiq_benchmark.o

all: bdev mpibm perf_singlepair.x perf_multipairs.x cp

bdev:
	cd $(DEV) && $(MAKE) -f Makefile
mpibm:
	cd $(BM)/mpi && $(MAKE) -f Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm *.x
	rm *.o

realclean: clean
	rm $(OBJ)
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

BENCHMARKS_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/benchmarks

cp:
	cp perf_singlepair.x $(BENCHMARKS_DIR)/multipaths/singlepair/
	cp perf_multipairs.x $(BENCHMARKS_DIR)/multipaths/multipairs/
