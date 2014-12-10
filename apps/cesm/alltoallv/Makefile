#CC = gcc
CC = g++

COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

INCLUDE = -I. 

CFLAGS  = $(COPT) $(INCLUDE)

LD = g++

LIBS    = -L$(MPICH2_PATH)/lib -l mpi -lm

OBJ = util.o topology.o datagen.o

all: util.o topology.o datagen.o patterngen.x pathreconstruct.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

rm:
	rm *.o
	rm *.x
