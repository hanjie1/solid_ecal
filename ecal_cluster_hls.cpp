#include "ecal_cluster_hls.h"

ap_uint<8> disc_cluster(cluster_t ac, ap_uint<16> cluster_threshold)
{
  ap_uint<8> result = 0;

  // discriminate and report correct 4ns time bin that threshold crossing happened
  if(ac.e >= cluster_threshold)
    result[ac.t] = 1;

  return result;
}

// ecal_cluster_hls:
// - hit_dt: maximum time difference (in +/-4ns ticks) from seed hit required to accept adjacent spacial hit into cluster
// - seed_threshold: minimum hit energy required for central hit of cluster position to allow a cluster to be formed
// - cluster_threshold: mimimum cluster energy required to generate a trigger
// - s_fadc_hits_pre: FADC hit stream input (from VXS and fiber) of all fadc hits that can be used to perform cluster finding from the previous frame
// - s_fadc_hits: FADC hit stream input (from VXS and fiber) of all fadc hits that can be used to perform cluster finding
// - s_trigger: trigger stream output
// - s_cluster_all: cluster stream output
void ecal_cluster_hls(
    ap_uint<3> hit_dt,
    ap_uint<13> seed_threshold,
    ap_uint<16> cluster_threshold,
    hls::stream<fadc_hits_t> &s_fadc_hits,
    hls::stream<trigger_t> &s_trigger,
    hls::stream<cluster_all_t> &s_cluster_all
  )
{
  fadc_hits_t fadc_hits = s_fadc_hits.read();
#ifndef __SYNTHESIS__
  // Initialize for simulation only (creates a problem for synthesis scheduling)
  static fadc_hits_t fadc_hits_pre = {{{0,0}},{{0,0}},{{0,0}}};
#else
  static fadc_hits_t fadc_hits_pre;
#endif
  ap_uint<8> ac_disc[N_CHAN_SEC];
  trigger_t trigger = {0};
  cluster_all_t allc;
  
  for(int ch=0; ch<N_CHAN_SEC;ch++){
      hit_t nearby_hit_pre[7];
      hit_t nearby_hit[7];

      ap_uint<3> edge=Find_block(ch,2);
   
      if(edge==0){
        for(int in=0; in<7; in++){
 	   int nearby_ch = Find_nearby(ch, in);

           nearby_hit_pre[in].e=0;
           nearby_hit_pre[in].t=0;
           nearby_hit[in].e=0;
           nearby_hit[in].t=0;

           if(nearby_ch>=0){
	      nearby_hit_pre[in].e=fadc_hits_pre.vxs_ch[nearby_ch].e;
	      nearby_hit_pre[in].t=fadc_hits_pre.vxs_ch[nearby_ch].t;
              nearby_hit[in].e=fadc_hits.vxs_ch[nearby_ch].e;
              nearby_hit[in].t=fadc_hits.vxs_ch[nearby_ch].t;
	   }
         }
       }

      if(edge==1){
        for(int in=0; in<7; in++){
 	   int nearby_ch = Find_nearby(ch, in);

           nearby_hit_pre[in].e=0;
           nearby_hit_pre[in].t=0;
           nearby_hit[in].e=0;
           nearby_hit[in].t=0;

           if(nearby_ch>=0){
	     if(in==1 || in==3){
	      nearby_hit_pre[in].e=fadc_hits_pre.fiber_ch_l[nearby_ch].e;
	      nearby_hit_pre[in].t=fadc_hits_pre.fiber_ch_l[nearby_ch].t;
              nearby_hit[in].e=fadc_hits.fiber_ch_l[nearby_ch].e;
              nearby_hit[in].t=fadc_hits.fiber_ch_l[nearby_ch].t;
	     }
	     else{
	      nearby_hit_pre[in].e=fadc_hits_pre.vxs_ch[nearby_ch].e;
	      nearby_hit_pre[in].t=fadc_hits_pre.vxs_ch[nearby_ch].t;
              nearby_hit[in].e=fadc_hits.vxs_ch[nearby_ch].e;
              nearby_hit[in].t=fadc_hits.vxs_ch[nearby_ch].t;
	     }
	   }

         }
       }
      

      allc.c[ch] = Find_cluster(nearby_hit_pre, nearby_hit,hit_dt, seed_threshold, Find_block(ch,0), Find_block(ch,1));
  }
     
  // save the previous fadc_hits
  fadc_hits_pre = fadc_hits;

#ifndef __SYNTHESIS__
  int nclust = 0;
  for(int ch=0; ch<N_CHAN_SEC;ch++){
    if(allc.c[ch].nhits>1)
      nclust++;
  }
  printf("nclust: %d\n",nclust);
#endif

  s_cluster_all.write(allc);

  for(int ii=0; ii<N_CHAN_SEC;ii++){
     ac_disc[ii]=disc_cluster(allc.c[ii],cluster_threshold); 
  }
  
  // 'or' together result from all channels, for each possible 4ns time bin
  for(int ii=0;ii<N_CHAN_SEC;ii++)
    trigger.trig |= ac_disc[ii];
  
  // write trigger result
  s_trigger.write(trigger);

  return;
}

// for a given channel number, return the nx and ny of the channel map 
ap_uint<5> Find_block(ap_uint<8> ch, ap_uint<1> dim){    
  ap_uint<5> nx=0; 
  ap_uint<5> ny=0;
  ap_uint<3> edge=0;

  int slot = ch%16;  // slot number, start from 0
  int ich = ch-slot*16; // channel number inside a fadc, start from 0

  nx=block_map[slot][ich].nx;
  ny=block_map[slot][ich].ny;
  edge=block_map[slot][ich].edge;

  switch(dim){
    case 0: return nx;
    case 1: return ny;
    case 2: return edge;
  }

}

// for a given (x,y), return the channel number
int Find_channel(ap_uint<5> nx, ap_uint<5> ny, ap_uint<3> edge){
  int slot=0, ich=0;
  int ch = -1;

  if(edge==1) return (nx-1);

  for(int ii=0;ii<10;ii++){
   for(int jj=0;jj<16;jj++){
      if( (block_map[ii][jj].nx==nx) && (block_map[ii][jj].ny==ny) ){
	   slot = ii;
	   ich = jj;
	   ch = 16*slot+ich;
      }
   }
  }

  return ch;
}
   
int Find_nearby(ap_uint<8> ch, ap_uint<3> ii){
     ap_uint<5> nx=0, ny=0;
     ap_uint<2> edge=0;
     nx = Find_block(ch,0);
     ny = Find_block(ch,1);
     edge = Find_block(ch,2);

     if(nx<1 || ny<1){
#ifndef __SYNTHESIS__
        printf("couldn't find the block number for chan %d\n",ch.to_uint());
#endif
        return -1;
     }

     switch(ii){
       case 0: return ch; // middle
       case 1: return Find_channel(nx-1, ny-1, edge); // left up
       case 2: return Find_channel(nx-1, ny, edge);   // left down
       case 3: return Find_channel(nx, ny-1, edge);   // middle up
       case 4: return Find_channel(nx, ny+1, edge);   // middle down
       case 5: return Find_channel(nx+1, ny, edge);   // right up
       case 6: return Find_channel(nx+1, ny+1, edge); // right down
     }


     return -1;
}

ap_uint<1> hit_coin(ap_uint<3> t1, ap_uint<3> t2, ap_uint<3> dt) {
  ap_uint<3> diff = (t1<t2) ? (t2-t1) : (t1-t2);
  return (diff<=dt) ? 1 : 0;
}

cluster_t Find_cluster(
    hit_t prehits[7], hit_t curhits[7],
    ap_uint<3> hit_dt, ap_uint<13> seed_threshold,
    ap_uint<5> x, ap_uint<4> y
  ){
     ap_uint<4> t = 0;
     ap_uint<13> e_array[7];
     ap_uint<7> hits = 0;

     if(prehits[0].e>=seed_threshold && prehits[0].t>=4){
       t = prehits[0].t;    // map pre time 4 to 7 -> 4 to 7 (unchanged)
       e_array[0] = prehits[0].e;
       hits[0] = 1;
     }
     else if(curhits[0].e>=seed_threshold && curhits[0].t<4){
       t = curhits[0].t+8;  // map cur time 0 to 3 -> 8 to 11 (move to time after pre hit window)
       e_array[0] = curhits[0].e;
       hits[0] = 1;
     }
     

     for(int ii=1; ii<7; ii++){
       e_array[ii] = 0;

       if(curhits[ii].e && hit_coin(t, 8+curhits[ii].t, hit_dt))
       {
         e_array[ii] = curhits[ii].e;
         hits[ii] = 1;
       }

       if(prehits[ii].e && hit_coin(t, prehits[ii].t, hit_dt))
       {
         e_array[ii] = prehits[ii].e;
         hits[ii] = 1;
       }

       if(e_array[ii] >= e_array[0])
         hits[0] = 0;
     }

     cluster_t cc;
     cc.e = 0;
     cc.nhits = 0;
     cc.t = t-4;  // map time 4 to 11 -> 0 to 7
     cc.x = x;
     cc.y = y;
     if(hits[0])
     {
       for(int ii=0; ii<7; ii++)
       {
         cc.e += e_array[ii];
         cc.nhits += hits[ii];
       }
     }
     return cc;

}

