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
STRUCT = $(DEV)/structures
YEN = $(DEV)/algorithms/yen

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(YEN)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(STRUCT)/path.o $(STRUCT)/job.o $(UTIL)/util.o $(YEN)/YenTopKShortestPathsAlg.o $(YEN)/Graph.o $(YEN)/DijkstraShortestPathAlg.o $(YEN)/yen.o

all: test_yen.x

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
