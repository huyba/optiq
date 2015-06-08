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
MOD = $(DEV)/model
ALG = $(DEV)/algorithms
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(MOD) -I$(ALG) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV) -I$(BM)/mpi -I$(BM)/optiq/
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(MOD)/*.o $(ALG)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o $(DEV)/optiq.o $(BM)/mpi/*.o $(BM)/optiq/optiq_benchmark.o

all: bdev mpibm perf_bypathlen.x perf_jobs_fromfile.x extract_result.x groupdist.x perf_jobs_cesm.x cp

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

M = mira

BENCHMARKS_DIR = /gpfs/$(M)-fs0/projects/Performance/abui/optiq/benchmarks

cp:
	cp groupdist.x $(BENCHMARKS_DIR)/stdperf/groupdist/
	#cp groupdist_fromfile.x $(BENCHMARKS_DIR)/stdperf/groupdist/
	cp extract_result.x $(BENCHMARKS_DIR)/stdperf/extract/
	cp perf_bypathlen.x $(BENCHMARKS_DIR)/stdperf/pathlen/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload1
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload2
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload4
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload8
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload16
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/512/maxload32
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/512/all/
	cp perf_jobs_cesm.x     $(BENCHMARKS_DIR)/stdperf/single/optimization/512/msg/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/1k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/multiranks/heuristics/1k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/multisize/1k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/all/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/maxpath4/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/maxpath8/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/maxpath16/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/maxpath32/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/mxload3/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/mxload7/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/mxload10/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/mxload20/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/1k/mxload30/

	cp perf_jobs_cesm.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/cesm/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/multisize/2k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/all/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/maxpath4/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/maxpath8/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/maxpath16/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/maxpath32/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/extended/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload1/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload3/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload7/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload10/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload20/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/2k/mxload30/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload1/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload2/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload4/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload8/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload16/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/maxload32/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/multisize/4k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/4k/maxload1/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/4k/maxload4/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/4k/maxload8/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/4k/maxload16/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/all/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/extended/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/limited/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload1/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload3/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload7/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload10/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload20/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/4k/mxload30/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/2k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/4k/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/multisize/8k/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/8k/maxload4/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/8k/maxload8/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/8k/maxload16/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/heuristics/8k/maxload1/

	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/8k/all/
	cp perf_jobs_fromfile.x $(BENCHMARKS_DIR)/stdperf/single/optimization/8k/limited/
