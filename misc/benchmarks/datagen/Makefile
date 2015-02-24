ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3
endif

LD      = mpixlc_r

TOPO   = ../../core/system

CFLAGS  = $(COPT) -I. -I$(TOPO)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

obj = $(TOPO)/topology.o datagen.o

all: $(obj)  mpi copy

mpi: simple_comm.x simple_io.x 

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(obj)
	$(LD) $< $(obj) -o $@ $(LDFLAGS)

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

copy:
	cp simple_comm.x /gpfs/mira-fs0/projects/Performance/abui/optiq/datagen/
	cp simple_io.x /gpfs/mira-fs0/projects/Performance/abui/optiq/datagen/
