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
    hls::stream<trigger_t> &s_trigger//,
    //hls::stream<cluster_all_t> &s_cluster_all
  )
{
  // example trigger (is any FADC hit over threshold): reading fadc hit steam, and writing trigger stream
  fadc_hits_t fadc_hits = s_fadc_hits.read();
  ap_uint<8> fadc_disc[256];
  trigger_t trigger = {0};

  // discriminate each fadc channel
  for(int ch=0;ch<256;ch++)
    fadc_disc[ch] = disc(fadc_hits.vxs_ch[ch], seed_threshold);

  // 'or' together result from all channels, for each possible 4ns time bin
  for(int ch=0;ch<256;ch++)
    trigger.trig |= fadc_disc[ch];

  // write trigger result
  s_trigger.write(trigger);
}
