ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlcxx
  COPT    = -g -O3 #-DDEBUG
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),gxx)
    CC	    = g++
    COPT    = -g -Wall
    LG	    = g++
endif

OPTIQ = ../../..
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
STRUCTURE = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
YEN = $(DEV)/algorithms/yen

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCTURE) -I$(TOPOLOGY) -I$(YEN)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/util.o $(STRUCTURE)/path.o $(STRUCTURE)/heap_path.o $(TOPOLOGY)/topology.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/YenTopKShortestPathsAlg.o

all: $(OBJ) alltomany.o manytomany.o multipaths.o

%.o: %.c
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
