ifndef COMPILER
    COMPILER = gxx#XL
endif

ifeq ($(COMPILER),gxx)
    CC = g++
    COPT = -g -Wall
    LD = g++
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
  LD = mpixlc_r
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 #-DDEBUG
  LD = mpixlc_r
endif

MPICH2_PATH=/usr/local/mpich2

OPTIQ = ../../..
UTIL = $(OPTIQ)/dev/util
TOPOLOGY = $(OPTIQ)/dev/topology
STRUCTURE = $(OPTIQ)/dev/structures
HEURISTICS = $(OPTIQ)/dev/algorithms/heuristics
CESM = $(OPTIQ)/apps/cesm
CESM_COMM = $(CESM)/comm_graph

INCLUDE = -I. -I$(UTIL) -I$(TOPOLOGY) -I$(STRUCTURE) -I$(HEURISTICS) -I$(CESM_COMM)

CFLAGS = $(COPT) $(INCLUDE)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

INCLUDE += -I$(MPICH2_PATH)/include

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

OBJ = $(UTIL)/util.o $(TOPOLOGY)/topology.o $(STRUCTURE)/path.o $(STRUCTURE)/job.o $(STRUCTURE)/heap_path.o  $(HEURISTICS)/alltomany.o $(HEURISTICS)/manytomany.o $(HEURISTICS)/multibfs.o $(CESM_COMM)/cesm.o

all: $(OBJ) test_cesm.x test_alltomany.x test_manytomany.x copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

copy:
	cp test_alltomany.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/multibfs/
	cp test_manytomany.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/multibfs/

clean:
	rm $(OBJ)
	rm *.o *.x
