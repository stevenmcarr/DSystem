/* $Id: fd_types.h,v 1.11 1997/03/11 14:28:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*---------------------------------------------------------------------

    fd_types.h     Enumerated type definitions for Fortran D compiler
                   Struct for Fortran D command-line options

*/

#ifndef fortranD_types_h
#define fortranD_types_h

#ifndef forttypes_h
#include <libs/frontEnd/ast/forttypes.h>
#endif

#define MAXLOOP 10
#define DCMAX 5
#define DC_MAXDIM  5
#define NAME_LENGTH 40
#define DCMAX_LIST 100

enum FDtype { DECOMPOSITION = 1, ALIGN = 2, DISTRIBUTE = 3, NODEC = 4 };

enum dc_expr_type { value, variable, plus, irreg_val, unknown };

enum ALIGN_DISTRIB_STATE { ACTIVE = 1, INACTIVE = 2 };

#define FD_AlignIndexString "AlignIndex =  "
#define FortranDInfoString "FortranDInfo"
#define FD_CallSiteInfoString "CallSiteInformation"
#define FD_AlignTypeString "AlignEntryInfo =  "
#define FD_CallSiteGlobalsString "BeginCommonBlockInformation"
#define FD_CallSiteGlobalsStringEnd "EndCommonBlockInformation"

enum Fd_flags_index {
  Skip_irreg = 0,       /* Skip irregular part     */
  Code_before_reg = 1,  /* Generate code before regular compiler ? */
  Do_all_arrays = 2,    /* Process reg refs also ? */
  Split_comm = 3,       /* Split sends/recvs ?     */
  Save_irreg = 4,       /* Save .irreg.f file ?    */
  Gen_high_level = 5,   /* Generate high level / abstracted code */
  Do_interface = 6,     /* set up the Fortran D interface ?      */
  Do_mesg_vect = 7,
  Do_mesg_coalesce = 8,
  Do_vector_mesg_pipe = 9,
  Do_relax_owner = 10,
  Do_iteration_reorder = 11,
  Do_loop_fusion = 12,
  Do_loop_distribution = 13,
  Do_fine_grain_pipe = 14,
  Do_memory_order = 15,
  Do_coarse_grain_pipe = 16,
  Do_mesg_aggr = 17,
  Do_unbuffered_recv = 18,
  Do_unbuffered_send = 19,
  Instr_exec_time  = 20,	/* Instrumentation at program entry/exit */
  Instr_procedures = 21,	/* Instrumentation at proc entry/exit */
  Instr_loops      = 22,	/* Instrumentation at loop entry/exit */
  Instr_messages   = 23,	/* Instrumentation for individual messages */
  Instr_full_symb  = 24,	/* Instrumentation for all symbolic values */
  Fd_flags_cnt  = 25	/* NOT an actual flag, but the # of flags */
  };

extern const char *fd_flags_names[];

/* Structure holding the Fortran D command line information */

typedef struct Fd_opts
{ 
  char *pgm;                    /* program name */
  char *arch;                   /* target architecture */
  int ma_size;                  /* max mesg aggregate size */
  int cgp_size;                 /* coarse-grain pipelining strip size */
  int isend_size;               /* min unbuffered send size */
  int irecv_size;               /* min unbuffered recv size */
  int nprocs;                   /* # of processors */
  int wrk_array_sizes[TYPE_LAST]; /* Sizes of allocated work arrays */
  Boolean flags[Fd_flags_cnt];  /* command line flags */
} Fd_opts;

#endif
