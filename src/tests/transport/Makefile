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

SYSTEM   = ../../core/system
UTILS  = ../../core/utils
STRUCTURES = ../../core/structures
TRANSPORT = ../../engine/transport

INCLUDE = -I. -I../../# -I$(MPICH2_PATH)/include -I$(SYSTEM) -I$(STRUCTURES) -I$(UTILS) -I../../engine/ -I$(TRANSPORT)

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib -loptiq -L../../

CFLAGS += -DPROGRESS_THREAD

OBJ = 

libobj = $(UTILS)/util.o $(SYSTEM)/memory.o $(STRUCTURES)/flow.o $(STRUCTURES)/job.o ../../engine/message.o $(TRANSPORT)/pami/pami_transport.o $(TRANSPORT)/gni/gni_transport.o $(TRANSPORT)/nonblk_mpi/nonblk_mpi_transport.o $(TRANSPORT)/transport.o ../../engine/virtual_lane.o ../../liboptiq.a

all: lib $(OBJ) mpi copy

lib:
	cd ../../ && $(MAKE) -f Makefile

mpi: data_from_file.x pami_send_recv.x pami_forward.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

rm:
	cd ../../ && $(MAKE) -f Makefile rm
	rm *.x

rm: $(OBJ)
	rm $(OBJ)

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

copy:
	cp data_from_file.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/from_file/
	cp pami_send_recv.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/send_recv/
	cp pami_forward.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/forward/
