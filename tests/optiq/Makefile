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
MOD = $(DEV)/model
ALG = $(DEV)/algorithms
HEU = $(DEV)/algorithms/heuristics
YEN = $(DEV)/algorithms/yen
PAT = $(DEV)/comm_patterns
SCHED = $(DEV)/schedule
TRANS = $(DEV)/transport/pami_transport

CFLAGS  = $(COPT) -I. -I$(UTIL) -I$(STRUCT) -I$(TOPOLOGY) -I$(ALG) -I$(HEU) -I$(YEN) -I$(PAT) -I$(SCHED) -I$(TRANS) -I$(DEV) -I$(MOD)
LDFLAGS = $(COPT) -lpthread -lm -L/bgsys/drivers/ppcfloor/bgpm/lib

CFLAGS += -DPROGRESS_THREAD

OBJ = $(UTIL)/*.o $(STRUCT)/*.o $(TOPOLOGY)/*.o $(ALG)/*.o $(HEU)/*.o $(YEN)/*.o $(PAT)/*.o $(SCHED)/*.o $(TRANS)/*.o $(DEV)/optiq.o $(MOD)/model.o

all: bdev test_optiq_init.x test_optiq_alltoallv.x test_optiq_file.x test_optiq_pathbased.x test_optiq_kpaths.x test_optiq_jobs_from_file.x cp

bdev:
	cd $(DEV) && $(MAKE) -f Makefile

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(OBJ)
	$(LD) $< $(OBJ) -o $@ $(LDFLAGS)

clean: 
	rm *.x
	rm *.o

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*

TEST_DIR = /gpfs/vesta-fs0/projects/Performance/abui/optiq/tests

cp:
	mkdir -p $(TEST_DIR)
	mkdir -p $(TEST_DIR)/optiq/init/
	mkdir -p $(TEST_DIR)/optiq/alltoallv/
	mkdir -p $(TEST_DIR)/optiq/file/
	mkdir -p $(TEST_DIR)/optiq/pathbased/
	mkdir -p $(TEST_DIR)/optiq/kpaths/
	cp test_optiq_init.x $(TEST_DIR)/optiq/init/
	cp test_optiq_alltoallv.x $(TEST_DIR)/optiq/alltoallv/
	cp test_optiq_file.x $(TEST_DIR)/optiq/file/
	cp test_optiq_pathbased.x $(TEST_DIR)/optiq/pathbased/
	cp test_optiq_kpaths.x $(TEST_DIR)/optiq/kpaths/
	co test_optiq_jobs_from_file.x $(TEST_DIR)/optiq/jobfile/
