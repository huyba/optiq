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

TOPO=../../../core/system

INCLUDE = -I$(DMAPP_PATH)/include -I$(KRCA_PATH)/include -I$(UDREG_PATH)/include -I$(GNIH_PATH)/include -I$(PMI_PATH)/include -I$(TOPO)/
LIBS    = -L$(DMAPP_PATH)/lib64   -L$(UDREG_PATH)/lib64   -L$(UGNI_PATH)/lib64   -L$(PMI_PATH)/lib64 -ldmapp -ludreg -lugni -lpmi

CFLAGS  = $(COPT) $(INCLUDE)

LD      = $(CC)
LDFLAGS = $(COPT)

TESTS := $(patsubst %.c,%.x,$(wildcard *.c))
OBJ   := $(patsubst %.c,%.o,$(wildcard *.c))
ASM   := $(patsubst %.c,%.s,$(wildcard *.c))

all: rm $(TOPO)/topology.o coflow.x copy

rm:
	#rm -f *.x

tests: $(TESTS)

%.x: %.o
	$(LD) $(LDFLAGS) $(TOPO)/topology.o $< $(LIBS) -o $@

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
	cp coflow.x /scratch2/scratchdirs/huybui/optiq/coflow/
