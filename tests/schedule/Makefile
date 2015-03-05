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

OPTIQ = ../..
TEST = $(OPTIQ)/tests
DEV = $(OPTIQ)/dev
UTIL = $(DEV)/util
STRUCT = $(DEV)/structures
TOPOLOGY = $(DEV)/topology
ALG = $(DEV)/algorithms
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV) -I$(HEU) -I$(YEN)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(SCHED)/*.o $(TRANS)/*.o $(ALG)/*.o  $(HEU)/*.o $(YEN)/*.o $(DEV)/optiq.o

all:  mpi copy

mpi: bdev test_schedule_get_pair.x test_schedule_build.x test_schedule_notify_list.x test_schedule_dmode.x test_schedule_multiranks.x

bdev:
	cd $(DEV) && $(MAKE) -f Makefile

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

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests

copy:
	cp test_schedule_get_pair.x $(TEST_DIR)/schedule/
	cp test_schedule_build.x $(TEST_DIR)/schedule/build/
	cp test_schedule_notify_list.x $(TEST_DIR)/schedule/notifylist/
	cp test_schedule_dmode.x $(TEST_DIR)/schedule/mode/
	cp test_schedule_multiranks.x $(TEST_DIR)/schedule/multiranks/
