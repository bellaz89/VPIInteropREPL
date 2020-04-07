# GHDL interop test REPL

This is a test project to make a REPL using VPI interface.

## Requirements

1. GHDL or Icarus Verilog
2. boost-interprocess. This is necessary to make a high-performance context-switch.
3. a modern C++11 compliant compiler (either gcc or clang).

Note that only linux is supported

## Usage

A vhdl/verilog source has to be provided as well as the top component module. adder.vhd is provided as an example for GHDL:

```vhdl
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity adder is
   port
   (
      nibble1, nibble2 : in unsigned(3 downto 0); 
      sum       : out unsigned(3 downto 0); 
      carry_out : out std_logic
   );
end entity adder;
 
architecture behavioral of adder is
   signal temp : unsigned(4 downto 0); 
begin 
   temp <= ("0" & nibble1) + nibble2; 
   sum       <= temp(3 downto 0); 
   carry_out <= temp(4);
end architecture behavioral;
```

To start the REPL, type either:

```bash
make ghdl-mcode COMPONENT_SOURCE=adder.vhd COMPONENT_NAME=adder
make ghdl-gcc COMPONENT_SOURCE=adder.vhd COMPONENT_NAME=adder
make ghdl-llvm COMPONENT_SOURCE=adder.vhd COMPONENT_NAME=adder
```

An equivalent verilog source is provided for Icarus Verilog. This can be run with:

```bash
make iverilog COMPONENT_SOURCE=adder.v COMPONENT_NAME=adder
```

Then the REPL is opened.

Example commands:

```
> setInt adder.nibble1 3
> setBigInt adder.nibble2 0110
> sleep 1
> getLong adder.sum
 = 9
> setLong adder.nibble1 0
> sleep 2
> getInt adder.sum
 = 6
> getBigInt adder.sum
 = 00000110
> 
```

All the commands can be listed with the `help` command:

```
> help 

 Available commands:
   end : stop the simulation and exit
   eval : evaluate the module
   getBigInt : <signalName> get a big integer as binary
   getInt : <signalName> get an int
   getLong : <signalName> get a long
   help : this command
   printSignals : print signal names of the top module
   setBigInt : <signalName> <binary> set a big integer
   setInt : <signalName> <integer> set a integer
   setLong : <signalName> <integer> set a long
   sleep : <integer> sleep for a number of steps

> 
```

An useful command to list all available signals is `printSignals`:

```
> printSignals

 Signals of adder :
   adder.nibble1
   adder.nibble2
   adder.sum
   adder.carry_out
   adder.temp

> 
```

After the simulation is stopped with the `end` command a wavefile, test.vcd is generated.


![GTKWave screenshot](gtkwave.png)


