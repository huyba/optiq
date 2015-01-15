#CC = gcc
#CC = g++
CC = mpicxx

COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

INCLUDE = -I. -I$(MPICH2_PATH)/include

CFLAGS  = $(COPT) $(INCLUDE)

LD = mpixlc_r #g++

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

OBJ = util.o topology.o datagen.o multibfs.o

all: $(OBJ) test_multibfs.x copy

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

copy:
	cp test_multibfs.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/multibfs/

rm:
	rm *.o
	rm *.x
