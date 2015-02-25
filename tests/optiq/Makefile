ifndef COMPILER
  COMPILER = XL
endif

ifeq ($(COMPILER),GNU)
  CC      = mpicc
  COPT    = -g -Wall -std=gnu99 -O2 -mcpu=a2 -mtune=a2
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),XL)
  CC      = mpicxx
  COPT    = -g -O3 #-DDEBUG
  LD	  = mpixlc_r
endif

ifeq ($(COMPILER),gxx)
    CC	    = g++
    COPT    = -g -Wall
    LG	    = g++
endif

OPTIQ = ../..
DEV = $(OPTIQ)/dev
TESTS = $(OPTIQ)/tests
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o $(DEV)/optiq.o

all: bdev test_optiq_init.x test_optiq_alltoallv.x cp

bdev:
	cd $(DEV) && $(MAKE) -f Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean: $(OBJ)
	rm $(OBJ)
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests

cp:
	cp test_optiq_init.x $(TEST_DIR)/optiq/init/
	cp test_optiq_alltoallv.x $(TEST_DIR)/optiq/alltoallv/
