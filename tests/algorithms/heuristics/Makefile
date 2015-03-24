ifndef COMPILER
    COMPILER = XL
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
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
TOPOLOGY = $(DEV)/topology
STRUCTURE = $(DEV)/structures
HEURISTICS = $(DEV)/algorithms/heuristics
PAT = $(DEV)/comm_patterns
YEN = $(DEV)/algorithms/yen

INCLUDE = -I. -I$(UTIL) -I$(TOPOLOGY) -I$(STRUCTURE) -I$(HEURISTICS) -I$(PAT) -I$(YEN)

CFLAGS = $(COPT) $(INCLUDE)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

INCLUDE += -I$(MPICH2_PATH)/include

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

YEN_OBJ = $(YEN)/yen.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/YenTopKShortestPathsAlg.o

OBJ = $(UTIL)/util.o $(TOPOLOGY)/*.o $(STRUCTURE)/path.o $(STRUCTURE)/job.o $(STRUCTURE)/heap_path.o  $(HEURISTICS)/*.o $(PAT)/cesm.o

all: $(OBJ) yen test_cesm.x test_alltomany.x test_manytomany.x test_mpiplus.x copy

yen:
	cd $(YEN)/ && $(MAKE) -f Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $(YEN_OBJ) $< $(LIBS) -o $@

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests

copy:
	cp test_alltomany.x $(TEST_DIR)/algorithms/heuristics
	cp test_manytomany.x $(TEST_DIR)/algorithms/heuristics
	cp test_mpiplus.x $(TEST_DIR)/algorithms/heuristics/mpiplus/
clean:
	rm *.x
	rm *.o
