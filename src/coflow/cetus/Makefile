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

TOPO   = ../../topo

CFLAGS  = $(COPT) -I. -I$(TOPO)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib 

CFLAGS += -DPROGRESS_THREAD

obj = $(TOPO)/topology.o

all: rm flow flowcpy

rm:
	rm -f *.o

flow: coflow.x

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
	cp coflow.x /gpfs/mira-fs0/projects/Performance/abui/microbm/coupling/coflow/
