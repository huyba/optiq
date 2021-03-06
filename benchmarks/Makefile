ifndef COMPILER
  COMPILER = gxx#XL
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

OPTIQ = ..
BENCHMARKS = $(OPTIQ)/benchmarks
UTIL = $(BENCHMARKS)/util
STRUCT = $(BENCHMARKS)/structures
TOPOLOGY = $(BENCHMARKS)/topology
HEU = $(BENCHMARKS)/algorithms/heuristics
YEN = $(BENCHMARKS)/algorithms/yen
PAT = $(BENCHMARKS)/comm_patterns
SCHED = $(BENCHMARKS)/schedule
TRANS = $(BENCHMARKS)/transport/pami_transport
HOPS = $(BENCHMARKS)/hops

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(PAT) -I$(SCHED) -I$(TRANS)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o

all: trans opt mpi sched bmhops

opt:
	cd $(BENCHMARKS)/optiq && $(MAKE) -f Makefile
mpi:
	cd $(BENCHMARKS)/mpi && $(MAKE) -f Makefile
util:
	cd $(UTIL) && $(MAKE) -f Makefile
struct:
	cd $(STRUCT) && $(MAKE) -f Makefile
topo:
	cd $(TOPOLOGY) && $(MAKE) -f Makefile
heu:
	cd $(HEU) && $(MAKE) -f Makefile
yen:
	cd $(YEN) && $(MAKE) -f Makefile
pat:
	cd $(PAT) && $(MAKE) -f Makefile
sched:
	cd $(SCHED) && $(MAKE) -f Makefile
trans:
	cd $(TRANS) && $(MAKE) -f Makefile
bmhops:
	cd $(HOPS) && $(MAKE) -f Makefile

clean: $(OBJ)
	rm $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

cp:
