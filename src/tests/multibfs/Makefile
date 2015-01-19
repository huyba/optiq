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

INCLUDE = -I.

CFLAGS = $(COPT) $(INCLUDE)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

INCLUDE += -I$(MPICH2_PATH)/include

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

OBJ = util.o topology.o datagen.o mtonbfs.o path.o heap_path.o

all: $(OBJ) test.x copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

copy:
	cp test.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/multibfs/

rm:
	rm *.o
	rm *.x
