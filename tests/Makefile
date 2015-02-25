ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 #-DDEBUG
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),gxx)
    CC	    = g++
    COPT    = -g -Wall
    LG	    = g++
endif

OPTIQ = ..
TESTS = $(OPTIQ)/tests
OPTEST = $(TEST)/optiq
UTIL = $(TESTS)/util
STRUCT = $(TESTS)/structures
TOPOLOGY = $(TESTS)/topology
HEU = $(TESTS)/algorithms/heuristics
YEN = $(TESTS)/algorithms/yen
PAT = $(TESTS)/comm_patterns
SCHED = $(TESTS)/schedule
TRANS = $(TESTS)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(PAT) -I$(SCHED) -I$(TRANS)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o

all: op struct topo heu yen trans

op:
	cd $(OPTEST) && $(MAKE) -f Makefile
struct:
	cd $(STRUCT) && $(MAKE) -f Makefile
topo:
	cd $(TOPOLOGY) && $(MAKE) -f Makefile
heu:
	cd $(HEU) && $(MAKE) -f Makefile
yen:
	cd $(YEN) && $(MAKE) -f Makefile
trans:
	cd $(TRANS) && $(MAKE) -f Makefile

clean: $(OBJ)
	rm $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

cp:
