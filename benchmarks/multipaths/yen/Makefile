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

OPTIQ = ../../..
DEV = $(OPTIQ)/dev
MOD = $(DEV)/model
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

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV) -I$(BM)/mpi -I$(BM)/optiq/ -I$(MOD)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(ALG)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o $(DEV)/optiq.o $(BM)/mpi/*.o $(BM)/optiq/optiq_benchmark.o $(MOD)/*.o

all: bdev mpibm yen_gen.x perf_optiq_kpaths.x yen_gen_parallel.x cp

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

BENCHMARKS_DIR = /gpfs/mira-fs0/projects/Performance/abui/optiq/benchmarks

cp:
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/512/
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/1k/
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/2k/
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/4k/
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/8k/
	cp yen_gen.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen/512/
	cp yen_gen_parallel.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen_parallel/1k/
	cp yen_gen_parallel.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen_parallel/2k/
	cp yen_gen_parallel.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen_parallel/4k/
	cp yen_gen_parallel.x $(BENCHMARKS_DIR)/multipaths/yen/yen_gen_parallel/8k/
	cp perf_optiq_kpaths.x $(BENCHMARKS_DIR)/multipaths/yen/perf/512/
	cp perf_optiq_kpaths.x $(BENCHMARKS_DIR)/multipaths/yen/perf/1k/
	cp perf_optiq_kpaths.x $(BENCHMARKS_DIR)/multipaths/yen/perf/2k/
	cp perf_optiq_kpaths.x $(BENCHMARKS_DIR)/multipaths/yen/perf/4k/
	cp perf_optiq_kpaths.x $(BENCHMARKS_DIR)/multipaths/yen/perf/8k/
