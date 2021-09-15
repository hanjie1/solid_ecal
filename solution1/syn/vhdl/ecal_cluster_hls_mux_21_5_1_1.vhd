-- ==============================================================
-- File generated on Wed Sep 15 10:43:53 EDT 2021
-- Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3 (64-bit)
-- SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
-- IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
-- Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
-- ==============================================================

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.NUMERIC_STD.all;

entity ecal_cluster_hls_mux_21_5_1_1 is
generic (
    ID            :integer := 0;
    NUM_STAGE     :integer := 1;
    din0_WIDTH       :integer := 32;
    din1_WIDTH       :integer := 32;
    din2_WIDTH       :integer := 32;
    dout_WIDTH        :integer := 32);
port (
    din0   :in  std_logic_vector(4 downto 0);
    din1   :in  std_logic_vector(4 downto 0);
    din2   :in  std_logic_vector(0 downto 0);
    dout     :out std_logic_vector(4 downto 0));
end entity;

architecture rtl of ecal_cluster_hls_mux_21_5_1_1 is
    -- puts internal signals
    signal sel    : std_logic_vector(0 downto 0);
    -- level 1 signals
    signal mux_1_0    : std_logic_vector(4 downto 0);
begin

sel <= din2;

-- Generate level 1 logic
mux_1_0 <= din0 when sel(0) = '0' else din1;

-- output logic
dout <= mux_1_0;

end architecture;
