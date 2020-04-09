ifdef GHDL
	GHDL=$(GHDL)
else
	GHDL=ghdl
endif

JAVA_HOME=$(shell readlink -f /usr/bin/java | sed "s:bin/java::")
CC=g++
CFLAGS=-std=c++11 -pedantic -Wall -Wextra -g
LDFLAGS=-lrt -lpthread
OUTPUT=./test.vcd
OUTPUT_TYPE=vcd
CLASSPATH=spinal.sim

SharedMemIface.o:
	$(CC) -c  -fPIC SharedMemIface.cpp -o SharedMemIface.o $(CFLAGS)

ghdl-mcode:
	ghdl-mcode -a $(COMPONENT_SOURCE)
	ghdl-mcode -e $(COMPONENT_NAME)
	ghdl-mcode --vpi-compile $(CC) -c VpiPlugin.cpp $(CFLAGS) -o VpiPlugin.o
	ghdl-mcode --vpi-link $(CC)  VpiPlugin.o $(LDFLAGS) $(CFLAGS) -o VpiPlugin.vpi
	$(CC) -c  main.cpp -DRUN_SIMULATOR_COMMAND='"make run_ghdl-mcode"' -o main.o $(CFLAGS)
	$(CC) -c  SharedMemIface.cpp -o SharedMemIface.o $(CFLAGS)
	$(CC) main.o SharedMemIface.o -o main $(CFLAGS) $(LDFLAGS)
	./main

run_ghdl-mcode:
	ghdl-mcode -r $(COMPONENT_NAME) --vpi=./VpiPlugin.vpi --$(OUTPUT_TYPE)=$(OUTPUT) &> /dev/null &

ghdl-gcc:
	ghdl-gcc -a $(COMPONENT_SOURCE)
	ghdl-gcc -e $(COMPONENT_NAME)
	ghdl-gcc --vpi-compile $(CC) -c VpiPlugin.cpp $(CFLAGS) -o VpiPlugin.o 
	ghdl-gcc --vpi-link $(CC) VpiPlugin.o $(LDFLAGS) $(CFLAGS) -o VpiPlugin.vpi
	$(CC) -c  main.cpp -DRUN_SIMULATOR_COMMAND='"make run_ghdl-gcc"' -o main.o $(CFLAGS)
	$(CC) -c  SharedMemIface.cpp -o SharedMemIface.o $(CFLAGS)
	$(CC) main.o SharedMemIface.o -o main $(CFLAGS) $(LDFLAGS)
	./main

run_ghdl-gcc:
	ghdl-gcc -r $(COMPONENT_NAME) --vpi=./VpiPlugin.vpi --$(OUTPUT_TYPE)=$(OUTPUT) &> /dev/null &

ghdl-llvm:
	ghdl-llvm -a $(COMPONENT_SOURCE)
	ghdl-llvm -e $(COMPONENT_NAME)
	ghdl-llvm --vpi-compile $(CC) -c VpiPlugin.cpp $(CFLAGS) -o VpiPlugin.o 
	ghdl-llvm --vpi-link $(CC) VpiPlugin.o $(LDFLAGS) $(CFLAGS) -o VpiPlugin.vpi
	$(CC) -c  main.cpp -DRUN_SIMULATOR_COMMAND='"make run_ghdl-llvm"' -o main.o $(CFLAGS)
	$(CC) -c  SharedMemIface.cpp -o SharedMemIface.o $(CFLAGS)
	$(CC) main.o SharedMemIface.o -o main $(CFLAGS) $(LDFLAGS)
	./main

run_ghdl-llvm:
	ghdl-llvm -r $(COMPONENT_NAME) --vpi=./VpiPlugin.vpi --$(OUTPUT_TYPE)=$(OUTPUT) &> /dev/null &

iverilog:
	$(CC) -c VpiPlugin.cpp $(CFLAGS) $(shell iverilog-vpi --cflags) -o VpiPlugin.o
	$(CC) VpiPlugin.o $(CFLAGS) $(shell iverilog-vpi --cflags) $(shell iverilog-vpi --lflags) $(shell iverilog-vpi --ldlibs) $(LDFLAGS) -shared -o VpiPlugin.vpi
	iverilog  $(COMPONENT_SOURCE) -s $(COMPONENT_NAME) -o iverilog_compiled.vvp
	$(CC) -c  main.cpp -DRUN_SIMULATOR_COMMAND='"make run_iverilog"' -o main.o $(CFLAGS)
	$(CC) -c  SharedMemIface.cpp -o SharedMemIface.o $(CFLAGS)
	$(CC) main.o SharedMemIface.o -o main $(CFLAGS) $(LDFLAGS)
	./main

run_iverilog:
	vvp -M. -mVpiPlugin iverilog_compiled.vvp -$(OUTPUT_TYPE) &> /dev/null &

SharedMemIface_wrap.cxx:
	swig -c++ -java SharedMemIface.i

SharedMemIface_wrap.o: SharedMemIface_wrap.cxx
	$(CC) -c -fPIC -I$(JAVA_HOME)include -I$(JAVA_HOME)include/linux SharedMemIface_wrap.cxx -o SharedMemIface_wrap.o

SharedMemIface.so: SharedMemIface_wrap.o SharedMemIface.o
	$(CC) -shared -fPIC -o SharedMemIface.so SharedMemIface_wrap.o SharedMemIface.o $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.vpi
	rm -f *.o
	rm -f *.out
	rm -f *.so
	rm -f simulation
	rm -f *.lst
	rm -f *.cf
	rm -f *.vcd
	rm -f *.cxx
	rm -f *.java

.PHONY: ghdl-mcode ghdl-gcc ghdl-llvm iverilog run_ghdl-mcode run_ghdl-gcc run_ghdl-llvm iverilog 
