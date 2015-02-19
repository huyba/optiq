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

OPTIQ = ../../../..
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
TOPO = $(DEV)/topology
STRUCT = $(DEV)/structures
ALG = $(DEV)/algorithms
CESM = $(OPTIQ)/apps/cesm
CESM_COMM = $(CESM)/comm_graph

INCLUDE = -I. -I$(UTIL) -I$(TOPO)/ -I$(STRUCT) -I$(ALG)/heuristics/ -I$(CESM_COMM)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o $(ALG)/heuristics/alltomany.o $(ALG)/heuristics/manytomany.o $(CESM_COMM)/cesm.o $(ALG)/heuristics/multibfs.o schedule.o comm_mem.o

all: $(OBJ) mpi copy

mpi: test_transport_alltomany.x test_transport_cesm.x

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

copy:
	cp test_transport_cesm.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/cesm/
	cp test_transport_cesm.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/cesm/message_size
	cp test_transport_alltomany.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/send_recv/
