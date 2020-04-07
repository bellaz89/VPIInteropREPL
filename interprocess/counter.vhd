library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity counter is
port(clk : in std_logic;
   reset : in std_logic;
   count : out std_logic_vector (2 downto 0));
end counter;
architecture behavioral of counter is
signal count_int : std_logic_vector(2 downto 0);
begin
process (reset, clk)
begin
 if reset = '1' then
  count_int <= "111";
 elsif clk'event and clk = '1' then
  if count_int <= "111" and count_int > "010" then
    count_int <= std_logic_vector(unsigned(count_int) - 1);
  else
    count_int <= "111";
  end if;
 end if;
end process;
count <= count_int;
end behavioral;

