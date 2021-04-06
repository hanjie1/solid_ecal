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
  // example trigger (is any FADC hit over threshold): reading fadc hit steam, and writing trigger stream
  fadc_hits_t fadc_hits = s_fadc_hits.read();
  static fadc_hits_t fadc_hits_pre;
  ap_uint<8> ac_disc[N_CHAN_SEC];
  trigger_t trigger = {0};
  cluster_all_t allc;
  cluster_t ac;
  static bool isfirst=true;

  int nclust=0;
  hit_t seed_hit;
  hit_t nearby_hit;
  
  for(int ch=0; ch<N_CHAN_SEC;ch++){
      seed_hit.e=0;
      seed_hit.t=0;
      ap_uint<1> ispre=0;
      if(fadc_hits_pre.vxs_ch[ch].e>seed_threshold && fadc_hits_pre.vxs_ch[ch].t>=4 && isfirst==false){
	   seed_hit.e=fadc_hits_pre.vxs_ch[ch].e;
	   seed_hit.t=fadc_hits_pre.vxs_ch[ch].t-4;
	   ispre=1;
      }

      if(fadc_hits.vxs_ch[ch].e>seed_threshold && fadc_hits.vxs_ch[ch].t<4){
	   seed_hit.e=fadc_hits.vxs_ch[ch].e;
	   seed_hit.t=fadc_hits.vxs_ch[ch].t+4;
	   ispre=0;
      }

      if(seed_hit.e>0){
	   ap_uint<8> nearby_ch=255;
	   hit_t nearby_hit_pre[6];
	   hit_t nearby_hit[6];
	   for(int in=0; in<6; in++){
	      nearby_ch = Find_nearby(ch, in);
	      if(nearby_ch<255){
	       nearby_hit_pre[in].e=fadc_hits_pre.vxs_ch[nearby_ch].e;
	       nearby_hit_pre[in].t=fadc_hits_pre.vxs_ch[nearby_ch].t;

	       nearby_hit[in].e=fadc_hits.vxs_ch[nearby_ch].e;
	       nearby_hit[in].t=fadc_hits.vxs_ch[nearby_ch].t;
	      }
	      else{
	       nearby_hit_pre[in].e=0;
	       nearby_hit_pre[in].t=0;

	       nearby_hit[in].e=0;
	       nearby_hit[in].t=0;
	      }
	   }

	   ac=Find_cluster(seed_hit, nearby_hit_pre, nearby_hit,hit_dt);

	   if(ac.nhits>1){
	     ac.t=seed_hit.t;
	     ac.x = Find_block(ch,0);	   	   
	     ac.y = Find_block(ch,1);	   	   

             nclust++;
	   }
	   else{ac.e=0; ac.t=0; ac.x=0; ac.y=0; ac.nhits=0;}
      }
      else{
	  ac.e=0; ac.t=0; ac.x=0; ac.y=0; ac.nhits=0;
      }
      allc.c[ch] = ac;

  }
     
  fadc_hits_pre = fadc_hits; 
  if(isfirst==true) isfirst=false;

#ifndef __SYNTHESIS__
  printf("nclust: %d\n",nclust);
#endif
  if(nclust==0)return;

  s_cluster_all.write(allc);

  for(int ii=0; ii<N_CHAN_SEC;ii++){
     ac_disc[ii]=disc_cluster(allc.c[ii],cluster_threshold); 
  }

  
  // 'or' together result from all channels, for each possible 4ns time bin
  for(int ii=0;ii<N_CHAN_SEC;ii++)
    trigger.trig |= ac_disc[ii];
  
  // write trigger result
  s_trigger.write(trigger);

  // save the previous fadc_hits
  return;
}

// build fadc map

typedef struct{
   ap_uint<5> nx;
   ap_uint<5> ny;
}block_coords;

block_coords block_map[10][16]={
       { {1,1},{1,2},{1,3},{1,4},{1,5},{2,1},{2,2},{2,3},{2,4},{2,5},{3,1},{3,2},{3,3},{3,4},{3,5},{3,6} },
       { {4,1},{4,2},{4,3},{4,4},{4,5},{4,6},{5,1},{5,2},{5,3},{5,4},{5,5},{5,6},{5,7},{6,1},{6,2},{6,3} },
       { {6,4},{6,5},{6,6},{6,7},{7,1},{7,2},{7,3},{7,4},{7,5},{7,6},{7,7},{7,8},{8,1},{8,2},{8,3},{8,4} },
       { {8,5},{8,6},{8,7},{8,8},{9,1},{9,2},{9,3},{9,4},{9,5},{9,6},{9,7},{9,8},{9,9},{10,1},{10,2},{10,3} },
       { {10,4},{10,5},{10,6},{10,7},{10,8},{10,9},{11,1},{11,2},{11,3},{11,4},{11,5},{11,6},{11,7},{11,8},{11,9},{11,10} },
       { {12,1},{12,2},{12,3},{12,4},{12,5},{12,6},{12,7},{12,8},{12,9},{12,10},{13,1},{13,2},{13,3},{13,4},{13,5},{13,6} },
       { {13,7},{13,8},{13,9},{13,10},{13,11},{14,1},{14,2},{14,3},{14,4},{14,5},{14,6},{14,7},{14,8},{14,9},{14,10},{14,11} },
       { {15,2},{15,3},{15,4},{15,5},{15,6},{15,7},{15,8},{15,9},{15,10},{15,11},{15,12},{16,3},{16,4},{16,5},{16,6},{16,7} },
       { {16,8},{16,9},{16,10},{16,11},{16,12},{17,5},{17,6},{17,7},{17,8},{17,9},{17,10},{17,11},{17,12},{17,13},{18,9},{18,10} },
       { {18,11},{18,12},{18,13},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0} }
}; 

// for a given channel number, return the nx and ny of the channel map 
ap_uint<5> Find_block(ap_uint<8> ch, ap_uint<1> dim){    
  ap_uint<5> nx=0; 
  ap_uint<5> ny=0;

  int slot = ch%16;  // slot number, start from 0
  int ich = ch-slot*16; // channel number inside a fadc, start from 0

  nx=block_map[slot][ich].nx;
  ny=block_map[slot][ich].ny;

  if( dim==0 ) return nx;
  else return ny;

}

// for a given (x,y), return the channel number
ap_uint<8> Find_channel(ap_uint<5> nx, ap_uint<5> ny){
  int slot=0, ich=0;
  ap_uint<8> ch=255;

  int found=0;
  for(int ii=0;ii<10;ii++){
   for(int jj=0;jj<16;jj++){
      if( (block_map[ii][jj].nx==nx) && (block_map[ii][jj].ny==ny) ){
	   slot = ii;
	   ich = jj;
	   ch = 16*slot+ich;
	   return ch; 
      }
   }
  }

  return ch;
}
 
ap_uint<8> Find_nearby(ap_uint<8> ch, ap_uint<3> ii){
     ap_uint<5> nx=0, ny=0;
     nx = Find_block(ch,0);
     ny = Find_block(ch,1);

     if(nx<1 || ny<1){
#ifndef __SYNTHESIS__
        printf("couldn't find the block number for chan %d\n",ch.to_uint());
#endif
        return 255;
     }

     switch(ii){
       case 0: return Find_channel(nx-1, ny-1); // left up
       case 1: return Find_channel(nx-1, ny);   // left down
       case 2: return Find_channel(nx, ny-1);   // middle up
       case 3: return Find_channel(nx, ny+1);   // middle down
       case 4: return Find_channel(nx+1, ny);   // right up
       case 5: return Find_channel(nx+1, ny+1); // right down
     }

     return 255;
}

cluster_t Find_cluster(hit_t seed_hit, hit_t prehits[6], hit_t curhits[6], ap_uint<3> hit_dt){

     ap_uint<16> total_e=seed_hit.e;
     ap_uint<4>  nhits=1;

     bool found=true;
     for(int ii=0; ii<6; ii++){
	if(prehits[ii].e<=seed_hit.e && prehits[ii].e>0){
            int dt = (seed_hit.t - (prehits[ii].t-4))*4;  //ns
	    if( dt<=(hit_dt+1) && dt>=-(hit_dt+1)){
	       total_e = total_e+prehits[ii].e;
	       nhits++;
	    }
	}

	if(curhits[ii].e<=seed_hit.e && curhits[ii].e>0){
            int dt = (seed_hit.t - (curhits[ii].t+4))*4;  //ns
	    if( dt<=(hit_dt+1) && dt>=-(hit_dt+1)){
	       total_e = total_e+curhits[ii].e;
	       nhits++;
	    }
	}
	
     }

     cluster_t cc={0,0,0,0,0};
     if(nhits>1){
	cc.e=total_e;
	cc.nhits=nhits;
     }

     return cc;

}

