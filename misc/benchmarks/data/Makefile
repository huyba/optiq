CC      = CC
COPT    = -g -O2
FOPT    = -g -O2

# CSCS paths
DMAPP_PATH=/opt/cray/dmapp/default
GNIH_PATH=/opt/cray/gni-headers/default
UGNI_PATH=/opt/cray/ugni/default
UDREG_PATH=/opt/cray/udreg/default
PMI_PATH=/opt/cray/pmi/default

OPTIQ_TOPO=../topo

INCLUDE = -I$(DMAPP_PATH)/include -I$(UDREG_PATH)/include -I$(GNIH_PATH)/include -I$(PMI_PATH)/include -I$(OPTIQ_TOPO)/
LIBS    = -L$(DMAPP_PATH)/lib64   -L$(UDREG_PATH)/lib64   -L$(UGNI_PATH)/lib64   -L$(PMI_PATH)/lib64 -ldmapp -ludreg -lugni -lpmi

CFLAGS  = $(COPT) $(INCLUDE)

LD      = $(CC)
LDFLAGS = $(COPT)

TESTS := $(patsubst %.c,%.x,$(wildcard *.c))
OBJ   := $(patsubst %.c,%.o,$(wildcard *.c))
ASM   := $(patsubst %.c,%.s,$(wildcard *.c))

all: tests copy

OTIQ_OBJ := $(OPTIQ_TOPO)/topology.o

tests: $(OTIQ_OBJ) testtopo.x

%.x: %.o
	$(LD) $(LDFLAGS) $(OTIQ_OBJ) $< $(LIBS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.s: %.c
	$(CC) $(CFLAGS) -S $< -o $@

clean:
	$(RM) $(RMFLAGS) $(OBJ)

realclean: clean
	$(RM) $(RMFLAGS) $(TESTS)
	$(RM) $(RMFLAGS) $(ASM)

copy:
	cp testtopo.x /scratch2/scratchdirs/huybui/optiq/tests/
