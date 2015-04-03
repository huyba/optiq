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

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(YEN) -I$(HEU) -I$(DEV) -I$(TOPO)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(STRUCT)/*.o $(UTIL)/util.o $(YEN)/YenTopKShortestPathsAlg.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/yen.o $(HEU)/*.o $(TOPO)/*.o

all: test_yen.x test_yen_gen.x cp

optiq:
	cd $(DEV) && $(MAKE) -f Makefile

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean: $(OBJ)
	rm $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

cp:
	cp test_yen.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/algorithms/yen/
	cp test_yen_gen.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/algorithms/yen/gen/
