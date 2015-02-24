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
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o

all: util struct topo heu yen pat sched trans optiq.o 

util:
	cd $(UTIL) && $(MAKE) -f Makefile
struct:
	cd $(STRUCT) && $(MAKE) -f Makefile
topo:
	cd $(TOPOLOGY) && $(MAKE) -f Makefile
heu:
	cd $(HEU) && $(MAKE) -f Makefile
yen:
	cd $(YEN) && $(MAKE) -f Makefile
pat:
	cd $(PAT) && $(MAKE) -f Makefile
sched:
	cd $(SCHED) && $(MAKE) -f Makefile
trans:
	cd $(TRANS) && $(MAKE) -f Makefile

clean: $(OBJ)
	rm $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

cp:
