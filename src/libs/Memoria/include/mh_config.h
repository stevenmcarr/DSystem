/* $Id: mh_config.h,v 1.1 1994/01/18 15:22:32 memoria Exp $ */

/*************************************************************************

    mh_config.h:  Header file for machine configuration for memory 
                  optimizing compiler.

 ************************************************************************/

#ifndef mh_config_h
#define mh_config_h

typedef struct {
  int     max_regs,
          int_regs,
          pipe_length,
          double_fetches,
          double_regs,
          div_cycles,
          mul_cycles,
          add_cycles,
          min_flop_cycles,
          hit_cycles,
          miss_cycles,
          chow_alloc,
          opt_li_alloc,
          line,
          mult_accum,
          soft_div,
          logging,
          write_back,
          aggressive,
          prefetch_latency,
	  prefetch_buffer,
          write_allocate;
  float   beta_m;
  FILE    *logfile;
 } config_type;


EXTERN(void, mh_get_config,(config_type *config,char *filename));

#endif mh_config_h
