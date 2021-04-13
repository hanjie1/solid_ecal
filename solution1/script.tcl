############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2018 Xilinx, Inc. All Rights Reserved.
############################################################
open_project ecal_cluster_hls
set_top ecal_cluster_hls
add_files ecal_cluster_hls/ecal_cluster_hls.h
add_files ecal_cluster_hls/ecal_cluster_hls.cpp
add_files -tb ecal_cluster_hls/ecal_cluster_hls_tb.cpp
open_solution "solution1"
set_part {xc7vx550tffg1927-1} -tool vivado
create_clock -period 32 -name default
set_clock_uncertainty 4
source "./ecal_cluster_hls/solution1/directives.tcl"
csim_design -compiler gcc
csynth_design
cosim_design -compiler gcc -trace_level port -rtl vhdl
export_design -format ip_catalog
