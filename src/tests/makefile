ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3
endif

LD      = mpixlc_r

SYSTEM   = ../core/system
STRUCTURE = ../core/structures
UTILS  = ../core/utils

CFLAGS  = $(COPT) -I. -I.. -I$(SYSTEM) -I$(SYSTEM)/bgq -I$(SYSTEM)/xe6 -I$(SYSTEM)/xc30 -I$(UTILS) -I$(STRUCTURE)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

obj = $(UTILS)/util.o $(STRUCTURE)/flow.o #$(SYSTEM)/bgq/topology_bgq.o $(SYSTEM)/xc30/topology_xc30.o $(SYSTEM)/xe6/topology_xe6.o $(SYSTEM)/topology.o $(UTILS)/util.o ../optiq.o

all: $(obj)  mpi #copy

mpi: readflow.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(obj)
	$(LD) $< $(obj) -o $@ $(LDFLAGS)

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

copy:
	cp test_topo_bgq.x /gpfs/mira-fs0/projects/Performance/abui/optiq/tests/topo/
