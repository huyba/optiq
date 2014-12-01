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

SRC = ../../..
SYSTEM   = $(SRC)/core/system
UTILS  = $(SRC)/core/utils
STRUCTURES = $(SRC)/core/structures
TRANSPORT = $(SRC)/engine/transport

INCLUDE = -I. -I$(SRC) -I$(SYSTEM) -I$(UTILS) -I$(SRC)/engine/ -I$(STRUCTURES)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(SRC)/engine/message.o $(UTILS)/util.o $(SYSTEM)/memory.o $(STRUCTURES)/queue.o pami_transport.o

all: $(OBJ) mpi copy

mpi: send_recv.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

copy:
	cp send_recv.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/send_recv/
