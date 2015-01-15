#CC = gcc
CC = g++

COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

INCLUDE = -I. -I$(MPICH2_PATH)/include

CFLAGS  = $(COPT) $(INCLUDE)

LD = g++

LIBS    = -L$(MPICH2_PATH)/lib -lm -l mpi

OBJ = util.o topology.o datagen.o multibfs.o

all: $(OBJ) test_multibfs.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

rm:
	rm *.o
	rm *.x
