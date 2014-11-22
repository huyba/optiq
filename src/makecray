CC      = CC 
COPT    = -g -O2 #-std=gnu99
FOPT    = -g -O2

# CSCS paths
DMAPP_PATH=/opt/cray/dmapp/default
GNIH_PATH=/opt/cray/gni-headers/default
UGNI_PATH=/opt/cray/ugni/default
UDREG_PATH=/opt/cray/udreg/default
PMI_PATH=/opt/cray/pmi/default
KRCA_PATH=/opt/cray/krca/default

OPTIQ_SYSTEM=core/system
OPTIQ_STRUCTURE=core/structures
OPTIQ_UTILS=core/utils

INCLUDE = -I. -I$(DMAPP_PATH)/include -I$(KRCA_PATH)/include -I$(UDREG_PATH)/include -I$(GNIH_PATH)/include -I$(PMI_PATH)/include -I$(OPTIQ_SYSTEM) -I$(OPTIQ_STRUCTURE) -I$(OPTIQ_UTILS)
LIBS    = -L$(DMAPP_PATH)/lib64   -L$(UDREG_PATH)/lib64   -L$(UGNI_PATH)/lib64   -L$(PMI_PATH)/lib64 -ldmapp -ludreg -lugni -lpmi

CFLAGS  = $(COPT) $(INCLUDE)

LD      = $(CC)
LDFLAGS = $(COPT) $(INCLUDE)

TESTS := $(patsubst %.c,%.x,$(wildcard *.c))
OBJ   := $(patsubst %.c,%.o,$(wildcard *.c))
ASM   := $(patsubst %.c,%.s,$(wildcard *.c))

OPTIQ_OBJS := $(OPTIQ_UTILS)/util.o  $(OPTIQ_SYSTEM)/xc30/topology_xc30.o $(OPTIQ_SYSTEM)/xe6/topology_xe6.o $(OPTIQ_SYSTEM)/bgq/topology_bgq.o $(OPTIQ_SYSTEM)/topology.o  

all: $(OPTIQ_OBJS) core/system/topology.o optiq.o

tests: $(TESTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJ)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)
	$(RM) $(RMFLAGS) $(ASM)

