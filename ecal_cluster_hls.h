#ifndef ecal_cluster_h
#define ecal_cluster_h

#include <ap_int.h>
#include <hls_stream.h>

// hit_t:
// - every 32ns each fadc channel reports 13 bit energy, and 3 bit hit time (time offset in current 32ns clock: 0=0ns, 1=4ns, 2=8ns, ..., 7=28ns)
// - if the channel has no hit, then the energy, e, will be reported as 0
// - energy, e, will saturate at 8191 (e.g. if the FADC integral (after pedestal subtraction and gain) is greater than 8191, the FADC report 8191
typedef struct
{
  ap_uint<13> e;
  ap_uint<3> t;
} hit_t;


// fadc_hits_t:
// - contains 256 VXS channels worth + 32 left fiber + 32 right fiber of hit_t reported each 32ns
// - vxs_ch[  0] to vxs_ch[ 15]: VME slot 3, ch 0 to 15 FADC channels
//   vxs_ch[ 16] to vxs_ch[ 31]: VME slot 4, ch 0 to 15 FADC channels
//   ...
//   vxs_ch[112] to vxs_ch[127]: VME slot 10, ch 0 to 15 FADC channels
//   (VXS switch A & B are at VME slot positions 11,12, so the FADC never can be installed here)
//   vxs_ch[128] to vxs_ch[143]: VME slot 13, ch 0 to 15 FADC channels
//   vxs_ch[144] to vxs_ch[159]: VME slot 14, ch 0 to 15 FADC channels
//   ...
//   vxs_ch[240] to vxs_ch[255]: VME slot 20, ch 0 to 15 FADC channels
//
//   fiber_ch_l[0] to fiber_ch_l[31]: these come from adjacent sectors of ecal and needed for current sector to build (6+1) clusters for trigger
//   fiber_ch_r[0] to fiber_ch_r[31]  channel mapping is arbitrary and needs to be defined
#define N_CHAN_SEC 147   // number of fadc channels per sector
typedef struct
{
  hit_t vxs_ch[N_CHAN_SEC];
  hit_t fiber_ch_l[32];
  hit_t fiber_ch_r[32];
} fadc_hits_t;

// cluster_t:
// - recommended cluster structure to be reported for each possible cluster position.
// - idx: cluster index (map based - alternatively a 2d coordinate would be fine is easy to define
// - e: cluster measured energy (sum of up to 7 fadc_hit_t.e)
// - t: cluster time, timestamped from cluster central hit time appended to coarse 32ns frame counter: this timestamp spans 0 to 8188ns (0 to 2047 *4ns)
// - nhits: number of hit channels in cluster. can be useful to reject single channel noise hits
typedef struct
{
  ap_uint<5> x;
  ap_uint<4> y;
  ap_uint<16> e;
  ap_uint<3> t;
  ap_uint<3>  nhits;
} cluster_t;


typedef struct
{
  cluster_t c[N_CHAN_SEC];
} cluster_all_t;

// trigger_t:
// - code works with 32ns of data at a time. hits & trigger have 4ns resolution, so 8 trigger decisions per iteration are computed.
// - trig: [0]=>0ns, [1]=>4ns, [2]=>8ns, ..., [7]=28ns, when bit=0 no trigger, when bit=1 trigger
typedef struct
{
  ap_uint<8> trig;
} trigger_t;

// build fadc map

typedef struct{
   ap_uint<5> nx;
   ap_uint<5> ny;
   ap_uint<3> edge;
}block_coords;


// nx, ny, edge (edge=0: middle, edge=1: anticlockwise side, edge=2: clockwise side)
block_coords block_map[10][16]={
       { {1,1,1},{1,2,0},{1,3,0},{1,4,2},{1,5,3},{2,1,1},{2,2,0},{2,3,0},{2,4,0},{2,5,4},{3,1,1},{3,2,0},{3,3,0},{3,4,0},{3,5,2},{3,6,3} },
       { {4,1,1},{4,2,0},{4,3,0},{4,4,0},{4,5,0},{4,6,4},{5,1,1},{5,2,0},{5,3,0},{5,4,0},{5,5,0},{5,6,2},{5,7,3},{6,1,1},{6,2,0},{6,3,0} },
       { {6,4,0},{6,5,0},{6,6,0},{6,7,4},{7,1,1},{7,2,0},{7,3,0},{7,4,0},{7,5,0},{7,6,0},{7,7,2},{7,8,3},{8,1,1},{8,2,0},{8,3,0},{8,4,0} },
       { {8,5,0},{8,6,0},{8,7,0},{8,8,4},{9,1,1},{9,2,0},{9,3,0},{9,4,0},{9,5,0},{9,6,0},{9,7,0},{9,8,2},{9,9,3},{10,1,1},{10,2,0},{10,3,0} },
       { {10,4,0},{10,5,0},{10,6,0},{10,7,0},{10,8,0},{10,9,4},{11,1,1},{11,2,0},{11,3,0},{11,4,0},{11,5,0},{11,6,0},{11,7,0},{11,8,0},{11,9,2},{11,10,3} },
       { {12,1,1},{12,2,0},{12,3,0},{12,4,0},{12,5,0},{12,6,0},{12,7,0},{12,8,0},{12,9,0},{12,10,4},{13,1,1},{13,2,0},{13,3,0},{13,4,0},{13,5,0},{13,6,0} },
       { {13,7,0},{13,8,0},{13,9,0},{13,10,2},{13,11,3},{14,1,1},{14,2,0},{14,3,0},{14,4,0},{14,5,0},{14,6,0},{14,7,0},{14,8,0},{14,9,0},{14,10,0},{14,11,4} },
       { {15,2,0},{15,3,0},{15,4,0},{15,5,0},{15,6,0},{15,7,0},{15,8,0},{15,9,0},{15,10,0},{15,11,2},{15,12,3},{16,3,0},{16,4,0},{16,5,0},{16,6,0},{16,7,0} },
       { {16,8,0},{16,9,0},{16,10,0},{16,11,0},{16,12,4},{17,5,0},{17,6,0},{17,7,0},{17,8,0},{17,9,0},{17,10,0},{17,11,0},{17,12,2},{17,13,3},{18,9,0},{18,10,0} },
       { {18,11,0},{18,12,0},{18,13,4},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0} }
}; 

// map for right side fiber
typedef struct{
   ap_uint<4> slot;  // start from 0
   ap_uint<4> ch;    // start from 0
}RF_chmap;

RF_chmap RF_ch[27]={{0,4},{0,3},{0,9},{0,15},{0,14},{1,5},{1,12},{1,11},{2,3},{2,11},{2,10},{3,3},{3,12},{3,11},{4,5},{4,15},{4,14}
                    {5,9},{6,4},{6,3},{6,15},{7,10},{7,9},{8,4},{8,13},{8,12},{9,2}};


void ecal_cluster_hls(
    ap_uint<3> hit_dt,
    ap_uint<13> seed_threshold,
    ap_uint<16> cluster_threshold,
    hls::stream<fadc_hits_t> &s_fadc_hits,
    hls::stream<trigger_t> &s_trigger,
    hls::stream<cluster_all_t> &s_cluster_all
  );

ap_uint<5> Find_block(ap_uint<8> ch, ap_uint<1> dim);
int Find_channel(ap_uint<5> nx, ap_uint<5> ny);
int Find_nearby(ap_uint<8> ch, ap_uint<3> ii);
cluster_t Find_cluster(hit_t prehits[7], hit_t curhits[7], ap_uint<3> hit_dt, ap_uint<13> seed_threshold, ap_uint<5> x, ap_uint<4> y);


#endif
