ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpixlcxx
  COPT    = -g -O3 #-DDEBUG
endif

LD      = mpixlcxx_r

DEV = ../..
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(STRUCT)/path.o $(STRUCT)/job.o $(UTIL)/util.o YenTopKShortestPathsAlg.o Graph.o DijkstraShortestPathAlg.o

all: $(OBJ) yen.x

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
