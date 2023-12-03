# Root Makefile to call src make and tool make

all:
	$(MAKE) -C src all

sim:
	$(MAKE) -C src sim

test:
	@src/sim 256 32 4 gcc_trace.txt

clean:
	$(MAKE) -C src clean

clobber:
	$(MAKE) -C src clobber
