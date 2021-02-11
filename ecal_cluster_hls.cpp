#include "ecal_cluster_hls.h"

ap_uint<8> disc(hit_t hit, ap_uint<13> seed_threshold)
{
  ap_uint<8> result = 0;

  // discriminate and report correct 4ns time bin that threshold crossing happened
  if(hit.e >= seed_threshold)
    result[hit.t] = 1;

  return result;
}

// ecal_cluster_hls:
// - hit_dt: maximum time difference (in +/-4ns ticks) from seed hit required to accept adjacent spacial hit into cluster
// - seed_threshold: minimum hit energy required for central hit of cluster position to allow a cluster to be formed
// - cluster_threshold: mimimum cluster energy required to generate a trigger
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
  ap_uint<8> fadc_disc[256];
  trigger_t trigger = {0};

  // discriminate each fadc channel
  for(int ch=0;ch<256;ch++)
    fadc_disc[ch] = disc(fadc_hits.vxs_ch[ch], seed_threshold);

  // check if there is a cluster
  int nclust=0;
  cluster_all_t allc;
  for(int ch=0;ch<256;ch++){
    allc.c[nclust]={0};
    if( fadc_disc[ch]>0 && ch<147){
        int nx=0, ny=0;
        Find_block(ch,nx,ny);
        if(nx<1 || ny<1){ printf("couldn't find the block number for chan %d\n",ch);continue;}

	int ch_nearby[6]={-1};	
         
	ch_nearby[0] = Find_channel(nx-1, ny-1);  // left up
	ch_nearby[1] = Find_channel(nx-1, ny);  // left down
	ch_nearby[2] = Find_channel(nx, ny-1);  // middle up
	ch_nearby[3] = Find_channel(nx, ny+1);  // middle down
	ch_nearby[4] = Find_channel(nx+1, ny);  // right up
	ch_nearby[5] = Find_channel(nx+1, ny+1);  // right down

	ap_uint<13> c_e = fadc_hits.vxs_ch[ch].e;
        ap_uint<3> c_t = fadc_hits.vxs_ch[ch].t;
	int total_e=c_e;
	int nhits=1;
	// find the if the center block is maximum both in e and t (could use vector and sort function here, not sure which one will be faster)
	bool found=true;
	for(int nn=0; nn<6; nn++){
	   if(ch_nearby[nn]>=0 && ch_nearby[nn]<147){
	      ap_uint<13> tmp_e = fadc_hits.vxs_ch[ch_nearby[nn]].e;
	      ap_uint<3> tmp_t = fadc_hits.vxs_ch[ch_nearby[nn]].t;
	      double dt = tmp_t-c_t;
	      if(c_e>=tmp_e && dt>=0 && dt<=8)  {total_e=total_e+tmp_e; nhits++;}
	      else {found=false; break;}
	   }	
	}
	printf("11 e=%d, t=%d\n",c_e.to_uint(),c_t.to_uint());
	if(found){
	  printf("1 cluster at (%d,%d) e=%d, t=%d, n=%d\n",nx,ny,total_e,c_t.to_uint(),nhits);
	  cluster_t acluster;
	  acluster.x=nx;
	  acluster.y=ny;
	  acluster.e=total_e;
	  acluster.t=c_t;
	  acluster.nhits=nhits;
	  allc.c[nclust]=acluster;
	  nclust++;
	}
    }
  }

  if(nclust>0)s_cluster_all.write(allc);
  
  // 'or' together result from all channels, for each possible 4ns time bin
  for(int ch=0;ch<256;ch++)
    trigger.trig |= fadc_disc[ch];

  // write trigger result
  s_trigger.write(trigger);
}

// build fadc map
// for a given channel number, return the nx and ny of the channel map 

typedef struct{
   int nx;
   int ny;
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

void Find_block(int ch, int& nx, int& ny){
  nx=0; ny=0;

  int slot = ch%16;  // slot number, start from 0
  int ich = ch-slot*16; // channel number inside a fadc, start from 0

  nx=block_map[slot][ich].nx;
  ny=block_map[slot][ich].ny;
  return;
}

int Find_channel(int nx, int ny){
  int slot=0, ich=0;
  int ch=-1;

  int found=0;
  for(int ii=0;ii<10;ii++){
   found=0;
   for(int jj=0;jj<16;jj++){
      if( (block_map[ii][jj].nx==nx) && (block_map[ii][jj].ny==ny) ){
	   slot = ii;
	   ich = jj;
	   found=1;
	   break;
      }
   }
   if(found==1) break;
  }

  if(found=1) ch = 16*slot+ich;

  return ch;
}
