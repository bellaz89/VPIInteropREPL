ifdef GHDL
	GHDL=$(GHDL)
else
	GHDL=ghdl
endif

CC=g++
CFLAGS=-fPIC -std=c++11
LDFLAGS=-lboost_fiber -lboost_context -lpthread 

all:
	$(GHDL) -a $(COMPONENT_SOURCE)
	$(GHDL) --bind $(COMPONENT_NAME)
	$(CC) -c  ghdlIface.cpp -o ghdlIface.o $(CFLAGS)
	$(CC) -c  main.cpp -o main.o $(CFLAGS)
	$(CC)  main.o ghdlIface.o  -Wl,`ghdl --list-link $(COMPONENT_NAME)` $(CFLAGS) $(LDFLAGS) -o simulation
	./simulation

vpi_plugin:
	$(GHDL) --vpi-compile $(CC) -DVPI_ENTRY_POINT_PTR=$(ENTRY_POINT_PTR) -c -o vpi_plugin.o vpi_plugin.c $(CFLAGS)
	$(GHDL) --vpi-link $(CC) -o vpi_plugin.vpi vpi_plugin.o

clean:
	rm -f *.vpi
	rm -f *.o
	rm -f *.so
	rm -f simulation
	rm -f *.lst
	rm -f *.cf
	rm -f *.vcd

.PHONY: all vpi_plugin clean
