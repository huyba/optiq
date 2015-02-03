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

DEV = ../../../../dev

MULTIBFS = ../../multibfs

INCLUDE = -I. -I$(MULTIBFS) -I$(DEV)/util/ -I$(DEV)/topology/ -I$(DEV)/structures/

CFLAGS = $(COPT) $(INCLUDE) 
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = pami_transport.o $(DEV)/util/util.o $(DEV)/topology/topology.o $(MULTIBFS)/mtonbfs.o $(DEV)/structures/heap_path.o $(DEV)/structures/path.o

all: $(OBJ) mpi copy

mpi: test.x test_models.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean:
	rm $(OBJ) *.x *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
	$(RM) $(RMFLAGS) $(libobj)

copy:
	cp test.x /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/pami_transport/send_recv/

	cp test_models.x  /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests/models/
