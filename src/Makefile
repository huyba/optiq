ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 -DDEBUG
endif

LD      = mpixlc_r

SYSTEM   = core/system
UTILS  = core/utils
STRUCTURES = core/structures
TRANSPORT = engine/transport

CFLAGS  = $(COPT) -I. -I.. -I$(SYSTEM) -I$(UTILS) -I$(TRANSPORT) -I$(STRUCTURES) -I./engine
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTILS)/util.o $(UTILS)/test.o $(SYSTEM)/memory.o $(STRUCTURES)/flow.o $(STRUCTURES)/job.o engine/message.o $(TRANSPORT)/pami/pami_transport.o $(TRANSPORT)/gni/gni_transport.o $(TRANSPORT)/nonblk_mpi/nonblk_mpi_transport.o $(TRANSPORT)/transport.o engine/virtual_lane.o optiq.o

all: $(OBJ) liboptiq.a

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

liboptiq.a: $(OBJ)
	rm -f $@
	ar cq $@ $(OBJ)

clobj: $(OBJ)
	rm $(OBJ)
	rm liboptiq.a

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

