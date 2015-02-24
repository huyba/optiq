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

OPTIQ = ../../../
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
TOPO = $(DEV)/topology
STRUCT = $(DEV)/structures
ALG = $(DEV)/algorithms
<<<<<<< HEAD
CESM = $(OPTIQ)/apps/cesm
CESM_COMM = $(CESM)/comm_graph
YEN = $(ALG)/yen

INCLUDE = -I. -I$(UTIL) -I$(TOPO)/ -I$(STRUCT) -I$(ALG)/heuristics/ -I$(CESM_COMM) -I$(YEN)
=======
TRANSPORT = $(DEV)/transport/pami_transport


INCLUDE = -I. -I$(UTIL) -I$(TOPO)/ -I$(STRUCT) -I$(TRANSPORT)
>>>>>>> 80182b3166738a38816c3f6012da52ddc4c7de23

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

<<<<<<< HEAD
YEN_OBJ = $(YEN)/yen.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/YenTopKShortestPathsAlg.o

OBJ = pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o $(ALG)/heuristics/alltomany.o $(ALG)/heuristics/manytomany.o $(CESM_COMM)/cesm.o $(ALG)/heuristics/multibfs.o schedule.o comm_mem.o patterns.o $(ALG)/heuristics/multipaths.o

all: $(OBJ) yen mpi copy

mpi: test_transport_alltomany.x test_transport_cesm.x test_transport_patterns.x

yen:
	cd $(YEN)/ && $(MAKE) -f Makefile
=======
OBJ = $(TRANSPORT)/pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o schedule.o

all: $(OBJ) mpi copy

mpi: perf_rput.x perf_transport_cesm.x perf_transport_alltomany.x perf_transport_patterns.x
>>>>>>> 80182b3166738a38816c3f6012da52ddc4c7de23

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

<<<<<<< HEAD
%.x: %.o $(OBJ) $(YEN_OBJ)
	$(LD) $< $(OBJ) $(YEN_OBJ) -o $@ $(LDFLAGS)
=======
%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)
>>>>>>> 80182b3166738a38816c3f6012da52ddc4c7de23

clean:
	rm $(OBJ) *.x *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

<<<<<<< HEAD
copy:
	cp test_transport_cesm.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/cesm/
	cp test_transport_cesm.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/cesm/message_size
	cp test_transport_patterns.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/patterns/
	cp test_transport_alltomany.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/send_recv/
=======
TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq

copy:
	cp test_init.x $(TEST_DIR)/pami_transport/init
	cp test_rput.x $(TEST_DIR)/pami_transport/rput
	cp test_sendrecv.x $(TEST_DIR)/pami_transport/sendrecv
>>>>>>> 80182b3166738a38816c3f6012da52ddc4c7de23
