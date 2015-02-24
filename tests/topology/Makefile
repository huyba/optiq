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

OPTIQ = ../..
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
TOPO = $(DEV)/topology
STRUCT = $(DEV)/structures
PAT = $(DEV)/comm_patterns

INCLUDE = -I. -I$(UTIL) -I$(TOPO)/ -I$(STRUCT) -I$(PAT)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/path.o $(PAT)/patterns.o $(PAT)/cesm.o $(TOPO)/pathreconstruct.o

all: $(OBJ) mpi copy

mpi: test_pathreconstruct.x test_topology.x

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

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests

copy:
	cp test_pathreconstruct.x $(TEST_DIR)/topology
	cp test_topology.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/topology
