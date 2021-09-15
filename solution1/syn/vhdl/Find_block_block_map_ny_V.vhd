-- ==============================================================
-- File generated on Wed Sep 15 10:43:53 EDT 2021
-- Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
-- SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
-- IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
-- Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
-- ==============================================================
library ieee; 
use ieee.std_logic_1164.all; 
use ieee.std_logic_unsigned.all;

entity Find_block_block_map_ny_V_rom is 
    generic(
             DWIDTH     : integer := 4; 
             AWIDTH     : integer := 8; 
             MEM_SIZE    : integer := 160
    ); 
    port (
          addr0      : in std_logic_vector(AWIDTH-1 downto 0); 
          ce0       : in std_logic; 
          q0         : out std_logic_vector(DWIDTH-1 downto 0);
          clk       : in std_logic
    ); 
end entity; 


architecture rtl of Find_block_block_map_ny_V_rom is 

signal addr0_tmp : std_logic_vector(AWIDTH-1 downto 0); 
type mem_array is array (0 to MEM_SIZE-1) of std_logic_vector (DWIDTH-1 downto 0); 
signal mem : mem_array := (
    0 => "0001", 1 => "0010", 2 => "0011", 3 => "0100", 4 => "0101", 
    5 => "0001", 6 => "0010", 7 => "0011", 8 => "0100", 9 => "0101", 
    10 => "0001", 11 => "0010", 12 => "0011", 13 => "0100", 14 => "0101", 
    15 => "0110", 16 => "0001", 17 => "0010", 18 => "0011", 19 => "0100", 
    20 => "0101", 21 => "0110", 22 => "0001", 23 => "0010", 24 => "0011", 
    25 => "0100", 26 => "0101", 27 => "0110", 28 => "0111", 29 => "0001", 
    30 => "0010", 31 => "0011", 32 => "0100", 33 => "0101", 34 => "0110", 
    35 => "0111", 36 => "0001", 37 => "0010", 38 => "0011", 39 => "0100", 
    40 => "0101", 41 => "0110", 42 => "0111", 43 => "1000", 44 => "0001", 
    45 => "0010", 46 => "0011", 47 => "0100", 48 => "0101", 49 => "0110", 
    50 => "0111", 51 => "1000", 52 => "0001", 53 => "0010", 54 => "0011", 
    55 => "0100", 56 => "0101", 57 => "0110", 58 => "0111", 59 => "1000", 
    60 => "1001", 61 => "0001", 62 => "0010", 63 => "0011", 64 => "0100", 
    65 => "0101", 66 => "0110", 67 => "0111", 68 => "1000", 69 => "1001", 
    70 => "0001", 71 => "0010", 72 => "0011", 73 => "0100", 74 => "0101", 
    75 => "0110", 76 => "0111", 77 => "1000", 78 => "1001", 79 => "1010", 
    80 => "0001", 81 => "0010", 82 => "0011", 83 => "0100", 84 => "0101", 
    85 => "0110", 86 => "0111", 87 => "1000", 88 => "1001", 89 => "1010", 
    90 => "0001", 91 => "0010", 92 => "0011", 93 => "0100", 94 => "0101", 
    95 => "0110", 96 => "0111", 97 => "1000", 98 => "1001", 99 => "1010", 
    100 => "1011", 101 => "0001", 102 => "0010", 103 => "0011", 104 => "0100", 
    105 => "0101", 106 => "0110", 107 => "0111", 108 => "1000", 109 => "1001", 
    110 => "1010", 111 => "1011", 112 => "0010", 113 => "0011", 114 => "0100", 
    115 => "0101", 116 => "0110", 117 => "0111", 118 => "1000", 119 => "1001", 
    120 => "1010", 121 => "1011", 122 => "1100", 123 => "0011", 124 => "0100", 
    125 => "0101", 126 => "0110", 127 => "0111", 128 => "1000", 129 => "1001", 
    130 => "1010", 131 => "1011", 132 => "1100", 133 => "0101", 134 => "0110", 
    135 => "0111", 136 => "1000", 137 => "1001", 138 => "1010", 139 => "1011", 
    140 => "1100", 141 => "1101", 142 => "1001", 143 => "1010", 144 => "1011", 
    145 => "1100", 146 => "1101", 147 to 159=> "0000" );

attribute syn_rom_style : string;
attribute syn_rom_style of mem : signal is "select_rom";
attribute ROM_STYLE : string;
attribute ROM_STYLE of mem : signal is "distributed";

begin 


memory_access_guard_0: process (addr0) 
begin
      addr0_tmp <= addr0;
--synthesis translate_off
      if (CONV_INTEGER(addr0) > mem_size-1) then
           addr0_tmp <= (others => '0');
      else 
           addr0_tmp <= addr0;
      end if;
--synthesis translate_on
end process;

p_rom_access: process (clk)  
begin 
    if (clk'event and clk = '1') then
        if (ce0 = '1') then 
            q0 <= mem(CONV_INTEGER(addr0_tmp)); 
        end if;
    end if;
end process;

end rtl;

Library IEEE;
use IEEE.std_logic_1164.all;

entity Find_block_block_map_ny_V is
    generic (
        DataWidth : INTEGER := 4;
        AddressRange : INTEGER := 160;
        AddressWidth : INTEGER := 8);
    port (
        reset : IN STD_LOGIC;
        clk : IN STD_LOGIC;
        address0 : IN STD_LOGIC_VECTOR(AddressWidth - 1 DOWNTO 0);
        ce0 : IN STD_LOGIC;
        q0 : OUT STD_LOGIC_VECTOR(DataWidth - 1 DOWNTO 0));
end entity;

architecture arch of Find_block_block_map_ny_V is
    component Find_block_block_map_ny_V_rom is
        port (
            clk : IN STD_LOGIC;
            addr0 : IN STD_LOGIC_VECTOR;
            ce0 : IN STD_LOGIC;
            q0 : OUT STD_LOGIC_VECTOR);
    end component;



begin
    Find_block_block_map_ny_V_rom_U :  component Find_block_block_map_ny_V_rom
    port map (
        clk => clk,
        addr0 => address0,
        ce0 => ce0,
        q0 => q0);

end architecture;


