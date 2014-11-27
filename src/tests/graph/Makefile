#CC = gcc
CC = g++

COPT = -g -O2 -Wall #-std=c99

MPICH2_PATH=/usr/local/mpich2

UTILS = ../../core/utils
SYSTEM = ../../core/system
STRUCTURE = ../../core/structures
TRANSPORT = ../../engine/transport
ENGINE = ../../engine

INCLUDE = -I. -I$(MPICH2_PATH)/include -I$(SYSTEM) -I$(STRUCTURE) -I$(UTILS) -I$(ENGINE) -I$(TRANSPORT)

LD = g++

LIBS    = -L$(MPICH2_PATH)/lib -l mpi

OBJ = $(UTILS)/util.o $(SYSTEM)/memory.o $(ENGINE)/message.o $(STRUCTURE)/flow.o $(STRUCTURE)/job.o $(TRANSPORT)/pami/pami_transport.o $(TRANSPORT)/gni/gni_transport.o $(TRANSPORT)/nonblk_mpi/nonblk_mpi_transport.o $(TRANSPORT)/transport.o $(ENGINE)/virtual_lane.o

all: $(OBJ) coarsening.x

CFLAGS  = $(COPT) $(INCLUDE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o
	$(LD) $(OBJ) $< $(LIBS) -o $@

cleanall:
	rm $(OBJ)
	rm *.x

rm:
	rm $(OBJ)
	rm *.x
