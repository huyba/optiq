#CC = gcc
CC = g++

COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

OPTIQ = ../../..
UTIL = $(OPTIQ)/dev/util
TOPOLOGY = $(OPTIQ)/dev/topology

INCLUDE = -I. -I$(UTIL) -I$(TOPOLOGY)

CFLAGS  = $(COPT) $(INCLUDE)

LD = g++

LIBS    = -L$(MPICH2_PATH)/lib -lm #-l mpi

OBJ = datagen.o $(UTIL)/util.o $(TOPOLOGY)/topology.o

all: $(OBJ) patterngen.x  multibfs.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $(OBJ) $< $(LIBS) -o $@

clean:
	rm $(OBJ)
	rm *.x
	rm *.o
