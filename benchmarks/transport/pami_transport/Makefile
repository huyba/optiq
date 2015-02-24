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
TRANS = $(DEV)/transport/pami_transport
SCHED= $(DEV)/schedule

INCLUDE = -I. -I$(UTIL) -I$(TOPO) -I$(STRUCT) -I$(TRANS) -I$(SCHED)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(TRANS)/pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o $(SCHED)/schedule.o $(SCHED)/comm_mem.o

all: $(OBJ) mpi copy

mpi: perf_rput.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm $(OBJ) *.x *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

BENCHMARK_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/benchmarks

copy:
	cp perf_rput.x $(BENCHMARK_DIR)/pami_transport/rput
