/* $Id: fd_init.C,v 1.1 1997/03/11 14:28:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*---------------------------------------------------------------------

    fd_init.C     Initialize command-line options etc.

*/

#include <libs/support/misc/general.h>
#include <libs/fortD/misc/fd_types.h>

/*------------------ LOCAL DECLARATIONS ---------------------*/

EXTERN(void, init_fd_opts,   (Fd_opts *fd_opts));

/*------------------- LOCAL CONSTANT DEFINITIONS ------------*/

const char *fd_flags_names[] = {
  "skip_irreg",
  "code_before_reg",
  "do_all_arrays",
  "split_comm",
  "save_irreg",
  "gen_high_level",
  "do_interface",
  "do_mesg_vect",
  "do_mesg_coalesce",
  "do_vector_mesg_pipe",
  "do_relax_owner", 
  "do_iteration_reorder",
  "do_loop_fusion", 
  "do_loop_distribution",
  "do_fine_grain_pipe", 
  "do_memory_order", 
  "do_coarse_grain_pipe",
  "do_mesg_aggr", 
  "do_unbuffered_recv", 
  "do_unbuffered_send",
  "instr_exec_time",
  "instr_procedures",
  "instr_loops",
  "instr_messages"
  "instr_full_symbolics",
};


/*********************************************************************
 *
 * init_fd_opts()   Initialize command line options
 * 
 */
void
init_fd_opts(Fd_opts *fd)
{
  int i;

  fd->pgm = 0;
  fd->arch = 0;
  fd->ma_size = 0;
  fd->cgp_size = 0;
  fd->isend_size = 0;
  fd->irecv_size = 0;
  fd->nprocs = 4;                  // Default # of processors

  for (i = 0; i < TYPE_LAST; i++)
  {
    fd->wrk_array_sizes[i] = 0;
  }

  for (i = 0; i < Fd_flags_cnt; i++)
  {
    fd->flags[i] = false;
  }
}
