############################################################
## This file is generated automatically by Vivado HLS.
## Please DO NOT edit it.
## Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
############################################################
set_directive_pipeline -enable_flush "ecal_cluster_hls"
set_directive_data_pack "ecal_cluster_hls" s_fadc_hits
set_directive_array_partition -type complete -dim 1 "ecal_cluster_hls" fadc_disc_time
set_directive_interface -mode ap_stable "ecal_cluster_hls" hit_dt
set_directive_interface -mode ap_stable "ecal_cluster_hls" seed_threshold
set_directive_interface -mode ap_stable "ecal_cluster_hls" cluster_threshold
set_directive_array_partition -type complete -dim 1 "ecal_cluster_hls" fadc_hits.vxs_ch
set_directive_array_partition -type complete -dim 1 "ecal_cluster_hls" fadc_hits.fiber_ch_l
set_directive_array_partition -type complete -dim 1 "ecal_cluster_hls" fadc_hits.fiber_ch_r
set_directive_array_partition -type complete -dim 1 "ecal_cluster_hls" fadc_disc
