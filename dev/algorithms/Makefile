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

OPTIQ = ../..
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
STRUCTURE = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
MODEL = $(DEV)/model
YEN = $(DEV)/algorithms/yen
HEU = $(DEV)/algorithms/heuristics

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCTURE) -I$(TOPOLOGY) -I$(YEN) -I$(HEU) -I$(MODEL)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ =

all: byen heu algorithm.o

heu:
	cd $(HEU) && $(MAKE) -f Makefile
byen:
	cd $(YEN) && $(MAKE) -f Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

cp:
