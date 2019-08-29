#!/bin/sh

if [ $# -eq 0 ]
  then
    echo "No arguments supplied"
    echo "simple vhdl testbench generator for string based fsm"
    echo "usage: bash gen_tb.sh libre"
    echo "% bash gen_tb.sh Libre > fsm_tb.vhdl"
    echo "% ghdl -a fsm.vhdl && ghdl -e fsm && ghdl -a fsm_tb.vhdl && ghdl -e fsm_tb && ghdl -r fsm_tb"
    exit 0
fi

cat << EndOfMessage
--Standard Library
library ieee;
--Standard Packages
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity fsm_tb is
end fsm_tb;

architecture tb of fsm_tb is
  signal clk  : std_logic;
  signal rst  : std_logic;
  signal inp  : std_logic_vector(7 downto 0);
  signal lock : std_logic;
  signal oot  : std_logic;

  -- Simulation support
--  signal clock_ena      : boolean := false;

-------------------------------------------------------------------------------
-- Clock generator procedure
-------------------------------------------------------------------------------
--  procedure clock_gen(
--    signal clock_signal   : out std_logic;
--    signal clock_enable   : in  boolean
--  ) is
--  begin
--    loop
--      if not clock_enable then
--        wait until clock_enable;
--      end if;
--      wait for 5 ns;
--      clock_signal <= '1';
--      wait for 5 ns;
--      clock_signal <= '0';
--    end loop;
--  end;


-----------------------------------------------------------------------------
-- DUT
-----------------------------------------------------------------------------
begin
--  clock_gen(clk, clock_ena);
  dut : entity work.fsm
    port map (clk => clk, rst => rst, inp => inp, lock => lock, oot => oot);

  p_main : process
  begin
    -- init
    inp <= x"00";
    rst <= '1';
    report "resetting" severity note;
    clk <= '0'; wait for 5 ns; clk <= '1'; wait for 5 ns;
    rst <= '0';
    assert lock = '0' report "idle state broken" severity error;

EndOfMessage

foo=$1
for (( i=0; i<${#foo}; i++ )); do
    letter=`printf "%x" "'${foo:$i:1}"`
    echo "    -- ${foo:$i:1}"
    echo "    inp <= x\"${letter}\";"
    echo "    report \"setting value ${foo:$i:1}\" severity note;"
    echo "    clk <= '0'; wait for 5 ns; clk <= '1'; wait for 5 ns;"
    echo "    assert lock = '1' report \"fsm unlocked on known path\" severity error;"
    echo
done

cat << EndOfMessage
    clk <= '0'; wait for 5 ns; clk <= '1'; wait for 5 ns;
    assert lock = '0' report "fsm unlocked on known path" severity error;
    assert oot = '1' report "known path is unrecognized by fsm" severity error;

    assert false report "end of test" severity note;
    wait;
  end process;
end tb;
EndOfMessage
