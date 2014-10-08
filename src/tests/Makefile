CC      = CC
COPT    = -g -Wall -std=gnu99

LD      = CC

TOPO   = ../core/system

CFLAGS  = $(COPT) -I. -I$(TOPO)

obj = $(TOPO)/topology.o

all: $(obj)  tests copy

tests: readtopo.x

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.x: %.o $(obj)
	$(LD) $< $(obj) -o $@

clean:
	$(RM) $(RMFLAGS) *.o
	$(RM) $(RMFLAGS) *.lst

realclean: clean
	$(RM) $(RMFLAGS) *.x
	$(RM) $(RMFLAGS) core.*
