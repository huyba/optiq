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
STRUCT = $(DEV)/structures
TOPO = $(DEV)/topology
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(YEN) -I$(HEU) -I$(DEV) -I$(TOPO) -I$(PAT)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(STRUCT)/*.o $(UTIL)/util.o $(YEN)/YenTopKShortestPathsAlg.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/yen.o $(HEU)/*.o $(TOPO)/*.o $(PAT)/*.o

all: yen_gen.x cp

optiq:
	cd $(DEV) && $(MAKE) -f Makefile

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm *.x
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

BM = /gpfs/vesta-fs0/projects/Performance/abui/optiq/benchmarks

cp:
	cp yen_gen.x $(BM)/multipaths/yen/
