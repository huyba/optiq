#CC = gcc
#CC = g++
CC = mpicxx
COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

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

LD      = mpixlc_r #g++

OPTIQ = ../../..
UTIL = $(OPTIQ)/dev/util
TOPOLOGY = $(OPTIQ)/dev/topology
STRUCTURE = $(OPTIQ)/dev/structures

INCLUDE = -I. -I$(UTIL) -I$(TOPOLOGY) -I$(STRUCTURE)

CFLAGS = $(COPT) $(INCLUDE)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

INCLUDE += -I$(MPICH2_PATH)/include

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

OBJ = $(UTIL)/util.o $(TOPOLOGY)/topology.o $(STRUCTURE)/path.o $(STRUCTURE)/job.o $(STRUCTURE)/heap_path.o  multibfs.o

all: $(OBJ) test_multibfs.x copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

copy:
	cp test_multibfs.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/multibfs/

clean:
	rm *.o
	rm *.x
