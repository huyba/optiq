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
TRANSPORT = $(DEV)/transport/pami_transport


INCLUDE = -I. -I$(UTIL) -I$(TOPO)/ -I$(STRUCT) -I$(TRANSPORT)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(TRANSPORT)/pami_transport.o $(UTIL)/util.o $(TOPO)/topology.o $(STRUCT)/heap_path.o $(STRUCT)/path.o schedule.o

all: $(OBJ) mpi copy

mpi: perf_rput.x perf_transport_cesm.x perf_transport_alltomany.x perf_transport_patterns.x

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

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq

copy:
	cp test_init.x $(TEST_DIR)/pami_transport/init
	cp test_rput.x $(TEST_DIR)/pami_transport/rput
	cp test_sendrecv.x $(TEST_DIR)/pami_transport/sendrecv
