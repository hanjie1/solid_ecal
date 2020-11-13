#include <stdio.h>
#include <stdlib.h>
#include "ecal_cluster_hls.h"

int main(int argc, char *argv[])
{
  ap_uint<3> hit_dt = 0;
  ap_uint<13> seed_threshold = 2000;
  ap_uint<16> cluster_threshold = 0;
  hls::stream<fadc_hits_t> s_fadc_hits;
  hls::stream<trigger_t> s_trigger, s_trigger_verify;
  hls::stream<cluster_all_t> s_cluster_all;

  srand(10);

  // generate some random FADC hits
  fadc_hits_t fadc_hits;
  trigger_t trigger;
  for(int frame=0;frame<4;frame++)
  {
    trigger.trig = 0;
    for(int ch=0;ch<256;ch++)
    {
      if(ch<32)
      {
        fadc_hits.fiber_ch_l[ch].e = 0;
        fadc_hits.fiber_ch_l[ch].t = 0;
        fadc_hits.fiber_ch_r[ch].e = 0;
        fadc_hits.fiber_ch_r[ch].t = 0;
      }
      if((rand() % 100)<1)  // 1% hit chance
      {
        fadc_hits.vxs_ch[ch].e = rand() % 8192; // random hit energy
        fadc_hits.vxs_ch[ch].t = rand() % 8;    // random hit time (4ns)

        printf("fadc hit: ch=%d, e=%d, t=%d\n",
            ch,
            fadc_hits.vxs_ch[ch].e.to_uint(),
            fadc_hits.vxs_ch[ch].t.to_uint()*4+frame*32
          );

        if(fadc_hits.vxs_ch[ch].e >= seed_threshold)
          trigger.trig[fadc_hits.vxs_ch[ch].t] = 1;
      }
      else
      {
        fadc_hits.vxs_ch[ch].e = 0;
        fadc_hits.vxs_ch[ch].t = 0;
      }
    }
    s_fadc_hits.write(fadc_hits);
    s_trigger_verify.write(trigger);
  }

  while(!s_fadc_hits.empty())
  {
    ecal_cluster_hls(
        hit_dt,
        seed_threshold,
        cluster_threshold,
        s_fadc_hits,
        s_trigger//,
        //s_cluster_all
      );
  }

  int t32ns = 0;
  int nfails = 0;
  int ntrigger_empty = 0;
  int ntrigger_verify_empty = 0;
  while(!s_trigger.empty() || !s_trigger_verify.empty())
  {
    trigger_t trigger, trigger_verify;

    if(!s_trigger.empty())
      trigger = s_trigger.read();
    else
    {
      printf("Error: s_trigger_empty()\n");
      ntrigger_empty++;
      continue;
    }

    if(!s_trigger_verify.empty())
      trigger_verify = s_trigger_verify.read();
    else
    {
      printf("Error: s_trigger_verify()\n");
      ntrigger_verify_empty++;
      continue;
    }

    if(trigger.trig != trigger_verify.trig)
    {
      printf("trigger != trigger_verify: %s != %s, t32ns=%d\n",
          trigger.trig.to_string(8,false).c_str(),
          trigger_verify.trig.to_string(8,false).c_str(),
          t32ns
        );
      nfails++;
    }

    for(int i=0;i<8;i++)
    {
      if(trigger.trig[i])
        printf("Trigger found at T=%dns\n", t32ns*32+i*4);
    }
    t32ns++;
  }

  return nfails+ntrigger_empty+ntrigger_verify_empty;
}
