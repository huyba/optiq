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
DEV = ../../../dev
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures
SCHED = $(DEV)/schedule

CFLAGS  = $(COPT) -I. -I$(SCHED) -I$(STRUCT) -I$(UTIL)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

all: pami_transport.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o 
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean: $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

