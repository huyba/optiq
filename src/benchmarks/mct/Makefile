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

TOPO   = ../../core/system/
DATA_GEN = ../datagen/

CFLAGS  = $(COPT) -I. -I$(TOPO) -I$(TOPO)/bgq -I$(TOPO)/xe6 -I$(TOPO)/xc30 -I$(DATA_GEN)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib 

CFLAGS += -DPROGRESS_THREAD

obj = $(TOPO)/topology.o $(DATA_GEN)/datagen.o

all: rm flow flowcpy

rm:
	rm -f *.o

flow: mct.x

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

flowcpy:
	cp mct.x /gpfs/mira-fs0/projects/Performance/abui/optiq/mct/
