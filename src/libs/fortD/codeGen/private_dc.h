/* $Id: private_dc.h,v 1.31 1997/03/11 14:28:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-------------------------------------------------------------

    private_dc.h    Definitions for the Fortran D 
                    distributed-memory compiler


    Side Array usage in the Fortran D compiler:

      AST        Type         Storage
    --------------------------------------------
      loop       type_ref     Loop_list*
      loop       type_dc      Rsd_set_info*
      loop       type_fd      FortD_LI*

      stmt       type_ref     Rsd_vector*
      stmt       type_dc      Iter_set*
      stmt       type_fd      S_group*

      subscript  type_ref     Subs_list*
      subscript  type_dc      Rsd_set*

      id         type_levelv  Level_vector*
      id         type_fd      SNODE*

*/


#ifndef FD_CODEGEN
#define FD_CODEGEN

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astcons.h>
#include <include/frontEnd/astnode.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/asttree.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/moduleAnalysis/dependence/dependenceTest/rsd.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/el.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt.h>

#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/misc/FortDEnum.h>


/*------------------------- local definitions ------------------------*/

#define MAXSUBS 40            /* max # of refs coalesced into Rsd_set */
#define MAXREF 256            /* max # of Rsd_sets per loop           */
#define MAXIRREG 1000         /* max # of irregular references        */
#define MAX_SGROUP 256        /* max # of non-loop statement groups   */
#define MAX_SGROUP_SIZE 64    /* max # of stmts in statement group    */
#define MAX_PROC 4            /* max # of procs comm for 1 stencil ref */
#define MAXNEWVAR 256         /* max # of new scalar variables created */
#define MAXSTMTS 4000         /* max # of statements in program        */

/* TEMP_MAX is the upper bound on # of arrays, distributions, lhs, etc.   */
/* just for prototype, will make it unlimited in future using dynamic mem */
    
#define TEMP_MAX 1000
#define LOOPTOTAL 512        /* total # of loops allowed in program */
                             /* MAXLOOP = max loop depth, not # !!! */

#define MAX_PROC_DIM 4       /* max dimension of processor array    */
#define DEFAULT_PROC 4       /* default # of processors */

#define D_NOTEXIST -1
#define D_ALL 0
#define D_SIMPLE 1
#define D_SIMPLE1 2
#define D_SIMPLE2 3
#define D_NOTSIMPLE 4
#define DC_NBOUNDARY 1
#define DC_BOUNDARY 2
#define DT1  1
#define DT2  2

#define MAX_RED_AUX 4        /* max number of aux reduction vars  */
#define MAXCOMMENT 256       /* max length of Fortran D comment   */
#define MAXFDBUF   32        /* max number of Fortran D buffers   */
#define MAX_FGP    32        /* max number of fine-grain pipe loops  */

#define DECOMP_LEN 13        /* length of keyword "decomposition" */ 
#define ALIGN_LEN   5        /* length of keyword "align"         */
#define DIST_LEN   10        /* length of keyword "distribute"    */

/*----- size of buffer elements in bytes -----*/

#define IBUF_ELEM_SIZE       4
#define RBUF_ELEM_SIZE       4
#define DPBUF_ELEM_SIZE      8

/*----- Optimization Options ----*/

#define MIN_UNBUFFERED    32
#define MIN_BUF_SIZE      4
#define MAX_AGGR_SIZE     10000
#define COARSE_PIPE_SIZE  8
#define MSG_START_ID      111

/*----- Fortran D names ----*/

#define IBUF_NAME       "i$buf"
#define RBUF_NAME       "r$buf"
#define DPBUF_NAME      "dp$buf"

#define NPROC           "n$p"
#define MYPROC          "my$p"
#define MYPID           "my$pid"

#define DECOMP          "decomposition"
#define ALIGN           "align"
#define DISTRIBUTE      "distribute"

#define BUF_NAME        "buf"
#define UNBUF_NAME      "unbuf"

/*------------------------- enumerated types ------------------------*/


enum RANGETYPE     /* range type (lb = lower bound, ub = upper bound) */
{
  POS_POS = 1,     /* lb >= 0, ub >= 0     */
  NEG_NEG = 2,     /* lb  < 0, ub  < 0     */
  NEG_POS = 3      /* lb  < 0, ub >= 0     */
};

/**********************************************************/
/* description of the set of iterations executed locally  */
/**********************************************************/

typedef enum Iter_type
{
  Iter_simple,     /* no boundary conditions, only set used              */
  Iter_pre,        /* boundary condition, pre_set & set used             */
  Iter_post,       /* boundary condition, post_set & set used            */
  Iter_pre_only,   /* boundary condition, only pre_set used              */
  Iter_pre_post,   /* boundary condition, pre_set & post_set used        */
  Iter_all,        /* boundary condition, pre_set, post_set & set used   */
  Iter_post_only,  /* used during communications                         */
  Iter_mid_only    /* mid set only, used for boundary conditions         */
} Iter_type;

/********************************************************************/
/* description of the set of iterations executed locally by cyclic  */
/********************************************************************/

typedef enum Citer_type
{
  Citer_none,       /* not cyclic distribution                            */
  Citer_simple,     /* no boundary conditions                             */
  Citer_lo,         /* boundary condition for lower loop bound            */
  Citer_up,         /* boundary condition for upper loop bound            */
  Citer_all         /* boundary condition for both lower & upper bounds   */
} Citer_type;

/********************************************************************/
/* description of the set of iterations executed locally by user    */
/********************************************************************/

typedef enum Uiter_type
{
  Uiter_none,       /* not user distribution                              */
  Uiter_value       /* value based                                        */
} Uiter_type;


/*------------------------ structure definitions -----------------------*/

/**********************************************************/
/* structure used to block allocate memory for compiler   */
/**********************************************************/

typedef struct dc_mem
{
  int size;               /* number of bytes consumed in buffer */
  struct dc_mem *next;    /* pointer to next buffer list elem   */
  char *buf;              /* generic memory buffer              */
} dc_mem;


/**********************************************************/
/* structure used to store information about scalars i.e  */
/* scalar reductions and private scalars                  */
/**********************************************************/

typedef struct Reduc_set
{
  Reduc_type rtype;      /* the type of reduction                       */
  AST_INDEX lhs;         /* lhs involved in reduction                   */
  AST_INDEX ref;         /* rhs array used to build the iteration set   */
  AST_INDEX loop;        /* loop after which to place reduce call       */
  char* lhs_name;        /* name of lhs involved in reduction           */
  Boolean init;          /* initialization statements for reduction     */
  Boolean done;          /* reduction has been processes                */
  Boolean local;         /* reduction occurs locally, not in parallel   */
  AST_INDEX aux[MAX_RED_AUX];  /* auxiliary reduction vars (eg. location) */
} Reduc_set;
 

/***************************************************************/
/* structure for the list of iterations to be executed locally */
/***************************************************************/

typedef struct Iter_set
{
  Iter_type type[MAXLOOP];      /* boundary condition type for iter set  */
  int dim_num[MAXLOOP];         /* store the dimension that corresponds  */
                                /* to the loop index                     */
  Loop_list set;                /* standard iteration set                */
  Loop_list pre_set;            /* iter set for leftmost proc(s) in dim  */
  Loop_list post_set;           /* iter set for rightmost proc(s) in dim */
  int pre_idle[MAXLOOP];        /* # of leftmost proc(s) with no work    */
  int post_idle[MAXLOOP];       /* # of rightmost proc(s) with no work   */
  Citer_type cyclic[MAXLOOP];   /* boundary conditions for cyclic        */
  Uiter_type user[MAXLOOP];     /* boundary conditions for user          */
  Generic irr_decomp[MAXLOOP];  /* Specific info for irregular distributions */
  int bksize[MAXLOOP];          /* block size of loop if partitioned     */
  AST_INDEX lhs;                /* (virtual) lhs of stmt for iter set    */
  SNODE *lhs_sp;                /* symbol table entry of (virtual) lhs   */
  Subs_list *sinfo;             /* subscripts of (virtual) lhs           */

  Boolean lhs_private;          /* if lhs is a private variable          */
  Boolean reduction;            /* if stmt is involved in a reduction    */
  Boolean allproc;              /* statement is executed by all procs    */
  Boolean oneproc;              /* statement is executed by only 1 proc  */

  int proc[MAX_PROC_DIM];       /* if oneproc = true, coordinates of proc  */
  AST_INDEX private_use;        /* if private = true, AST of use for lhs   */
  Reduc_set *reduc_set;         /* if reduction = true, info for reduction */
  int nonlocal_refs;            /* # refs causing nonlocal accesses */
} Iter_set;


/************************************************************************/
/* data structure that stores rsd information for a particular variable */
/************************************************************************/

typedef struct Rsd_set
{
  Rsd_section *rs, *rs_pre, *rs_post, *rs_carried;
  AST_INDEX subs[MAXSUBS];
  Subs_list *sinfo[MAXSUBS];
  int num_subs;
  int build_level;
  Iter_set *iterset; 
  AST_INDEX lhs;
  SNODE *sp;         /* pointer to variable represented by RSD */
  Mesg_type mtype;   /* MESG_INDEP, MESG_CARRIED_ALL, MESG_CARRIED_PART */
  Comm_type ctype;   /* COMM_SHIFT, etc... */
  int num_merge;     /* # of RSDs from other arrays merged with us */ 
  AST_INDEX location;            /* estimated location for message */
  struct Rsd_set *rsd_merge;     /* next merged RSD set  */
  struct Rsd_set *rsd_next;      /* next RSD set in list */
  struct Rsd_set *rsd_top;       /* root of list of RSD sets */
} Rsd_set;


/************************************************************************/
/* data structure stored in the side array at the loop level            */
/* It contains rsd information                                          */
/************************************************************************/

typedef struct Rsd_set_info
{
  Rsd_set *rsd_s[MAXREF];
  int num_ref;
} Rsd_set_info;


/****************************************************************/
/* structure used to form groups of statements so that          */
/* statements in each group possess identical iteration sets    */
/****************************************************************/

typedef struct S_group
{
  Stmt_type stype;                     /* execution type of statement  */
  int size;                            /* # of statements in group     */
  Rsd_set_info *rset;                  /* Rsds for statements in group */
  Iter_set *iset;                      /* iterset for stmts in group   */
  AST_INDEX stmt[MAX_SGROUP_SIZE];     /* statements in group          */
  AST_INDEX guard;                     /* guard generated for sgroup   */
  int nonlocal_refs;                   /* # refs causing nonlocal accesses */
} S_group;

typedef struct Stmt_groups
{
  int group_num;                   /* number of statement groups   */
  S_group *groups[MAX_SGROUP];     /* actual statement groups      */
} Stmt_groups;


/*********************************************************************/
/* description of Fortran D loop info */
/*********************************************************************/

typedef struct FortD_LI
{ 
  Loop_type ltype;                 /* execution type of loop           */
  Boolean localized;               /* loop indices converted to local? */
  Boolean uniform;                 /* partition of all stmts uniform?  */
  int depth;                       /* depth of loop                    */
  Dist_type dist_type;             /* type of loop partition, if any   */
  Citer_type cyclic_type;          /* boundary conditions for cyclic   */
  Uiter_type user_type;            /* boundary conditions for user     */
  Stmt_groups sgroup;              /* list of statement groups in loop */
  Iter_set *iset;                  /* union of itersets for sgroup     */
  int bksize;                      /* size of partition, if any        */
  AST_INDEX guard;                 /* AST of guard generated for loop  */
  AST_INDEX glo_ast;               /* AST for global index if needed   */
  AST_INDEX loc_ast;               /* AST for local index if needed    */
  AST_INDEX init;                  /* AST of last initialization stmt  */

  int num_indep_send;              /* # of indep send/recv msgs        */
  int num_c_all_send;              /* # of carried-all send/recv msgs  */
  int num_c_part_send;             /* # of carried-part send/recv msgs */
  int num_indep_bcast;             /* # of indep broadcasts            */
  int num_c_all_bcast;             /* # of carried-all broadcasts      */
  int num_indep_gather;            /* # of gathers communicated        */
  int num_c_all_gather;            /* # of gathers communicated        */
  int num_reduc;                   /* # of reductions communicated     */
} FortD_LI;


/*********************************************************************/
/* description of data to be sent/received by processor for an array */
/*********************************************************************/

typedef struct RSD_PARTS
{
  int num_dim;
  int numprocs;
  int total_procs;
  int upper[MAX_PROC];              /* upper bound              */
  int lower[MAX_PROC];              /* lower bound              */
  int proc_offset[MAX_PROC];
  int inv_upper[MAX_PROC];
  int inv_lower[MAX_PROC];
  int inv_proc_offset[MAX_PROC];
  AST_INDEX sym_upper[MAX_PROC];    /* symbolic upper bound     */
  AST_INDEX sym_lower[MAX_PROC];    /* symbolic lower bound     */
  AST_INDEX sym[MAX_PROC];          /* symbolic position in RSD */
  AST_INDEX extent[MAX_PROC];       /* extent of symbolic RSD   */
} RSD_PARTS;


/*********************************************************************/
/* description of Fortran D variables */
/*********************************************************************/

typedef struct Var_info
{
  int num;                     /* # of new variables    */
  AST_INDEX name[MAXNEWVAR];   /* AST of new variables  */
  enum FORM type[MAXNEWVAR];   /* type of new variables */
} Var_info;


/*********************************************************************/
/* description of Fortran D buffer storage */
/*********************************************************************/

typedef struct Buf_info
{
  char *name;                /* name of buffer class   */
  int num;                   /* number of buffers used */
  int size[MAXFDBUF];        /* size of each buffer    */
  Boolean active[MAXFDBUF];  /* whether buffer active  */
} Buf_info;

typedef struct FortD_bufs
{
  Buf_info i;    /* INTEGER          */
  Buf_info r;    /* REAL             */
  Buf_info dp;   /* DOUBLE PRECISION */
} FortD_bufs;


/*********************************************************************/
/* description of Fortran D optimization options */
/*********************************************************************/

typedef struct FortD_opts
{
  Boolean mesg_vectorize;
  Boolean mesg_coalesce;
  Boolean mesg_aggregate;
  Boolean vect_mesg_pipe;
  Boolean unbuffered_send;
  Boolean unbuffered_recv;
  Boolean iter_reorder;
  Boolean fine_grain_pipe;
  Boolean coarse_grain_pipe;
  Boolean relax_owner_computes;

  Boolean loop_fuse;     /* fuse loops unless result sequentialized */
  Boolean loop_dist;     /* distribute loops for iterset conflicts  */
  Boolean memory_order;  /* permute loops to improve data locality  */

  int min_unbuffered_send;    /* limit for using unbuffered send */ 
  int min_unbuffered_recv;    /* limit for using unbuffered recv */ 
  int min_buf_size;      /* limit for buffering routine (vs explicit copy) */ 
  int max_aggr_size;     /* limit for applying message aggregation         */
  int pipe_size;         /* default granularity of coarse-grain pipelining */
} FortD_opts;


/*********************************************************************/
/* names of architecture-specific system calls */
/*********************************************************************/

typedef struct Fd_Syscalls {
  char *nproc;    /* name of system call to get nproc            */
  char *myproc;   /* name of system call to get myproc           */
  char *mypid;    /* name of system call to get mypid            */
  char *csend;    /* name of system call for buffered send       */
  char *crecv;    /* name of system call for buffered recv       */
  char *bcast;    /* name of system call for broadcast           */
  char *brecv;    /* name of system call to recv broadcast       */
  char *isend;    /* name of system call for unbuffered send     */
  char *irecv;    /* name of system call for unbuffered send     */
  char *msgwait;  /* name of system call for synching unbuf comm */
} Fd_Syscalls;


/******************************************************************/
/* this structure contains all the globals variables to be        */
/* used by the compiler                                           */
/******************************************************************/

typedef struct Dist_Globals
{
  PedInfo ped;     /* main PED structures     */
  AST_INDEX root;  /* root AST of the program */

  Boolean in_ped;      /* Fortran D compiler called from PED      */
  Boolean in_ded;      /* Fortran D compiler called from D editor */

  AST_INDEX fortD_ph;  /* placeholder for last Fortran D statement   */
  AST_INDEX irr_decls_node;  /* Fortran D/irreg declarations go here */

  char *nproc;         /* name of total # procs in output            */
  char *myproc;        /* name of my processor number in output      */
  char *mypid;         /* name of my processor id in output          */
  char *ret;           /* name of temp var for storing return codes  */

  Fd_Arch_type arch;     /* FortD target archs (FD_NX, FD_CMMD)      */
  Fd_Syscalls syscalls;  /* system calls for target architecture     */

  /* appears in symbolt.c */
  SNODE *ihash[NIHASH];
  SNODE *dist_arrays[TEMP_MAX];
  SNODE *decomps[TEMP_MAX];

  int numprocs;    /* # of procs to run the program on */
  int numdecomps;  /* # of decompositions declared     */
  int numglobals;  /* # of distributed arrays          */
  int numdoloops;  /* # of do loops in program         */

  AST_INDEX doloops[LOOPTOTAL];   /* positions of loops bottom up */
  Stmt_groups sgroup;             /* list of statement groups     */
  Rsd_set_info rsd_loop_list[MAXLOOP];  /* rsd side array phase   */

  int mtype;     /* message number for typed messages */

  /* these ast's store the info needed for sends       */
  /* and receives.  srproc = proc# to send and receive */

  AST_INDEX recv_stmts;   /* AST for receiving statements             */
  AST_INDEX send_stmts;   /* AST for sending statements               */
  AST_INDEX srproc;       /* AST for send/recv processor              */
  AST_INDEX msgsize;      /* AST for size of one array section        */
  AST_INDEX bufsize;      /* AST for total buffer size (after aggreg) */
  AST_INDEX bcast;        /* AST for broadcast                        */
  AST_INDEX loop_first;   /* AST for first statement in loop          */

  Boolean const_msize;  /* whether message size is a compile-time constant */

  /* AST providing name of starting array/buffer location & type  */
  AST_INDEX startpoint;   /* starting point of message */
  AST_INDEX bufpoint;     /* starting point of buffer  */
  AST_INDEX buftype;      /* type & name of buffer     */

  AST_INDEX recv_sync;    /* AST for synchronizing unbuffered receive */
  AST_INDEX send_sync;    /* AST for synchronizing unbuffered send    */
  AST_INDEX msg_id;       /* id for unbuffered message                */
  int msg_id_num;         /* number of id for unbuffered message      */

  AST_INDEX head_send;    /* AST for sends to be inserted at head of loop */
  AST_INDEX head_recv;    /* AST for irecvs to be posted at head of loop  */

  Boolean send_userbuf;   /* sending from user buffer      */
  Boolean recv_userbuf;   /* receiving from user buffer    */ 

  /* num_nodes to communicate with for each reference   */
  int num_nodes;

  /* num_nodes to communicate with for each dimension */
  int num_dim_nodes[DC_MAXDIM];
  RSD_PARTS rs_parts[MAX_PROC][DC_MAXDIM];
  int proc;

  int min[TEMP_MAX];     /* min & max used to store range of  */
  int max[TEMP_MAX];     /* active processors during mesg gen */

  Var_info vars;       /* Fortran D variables            */
  FortD_bufs bufs;     /* Fortran D buffers              */
  FortD_opts opts;     /* Fortran D optimization options */

                       /* Coarse-Grain Pipelining Info */
  int cgp_grain;         /* granularity of cgp (strip size)   */
  AST_INDEX cgp_size;    /* AST of strip size if nonconstant  */
  AST_INDEX cgp_up;      /* AST of upper bound if nonconstant */
  AST_INDEX cgp_ivar;    /* AST of stripped loop, e.g. "i"    */
  AST_INDEX cgp_ivar2;   /* AST of iterator loop, e.g. "i$"   */

  AST_INDEX fgp[MAX_FGP]; /* list of fine-grain pipelined loops */
  int fgp_idx;            /* # of fine-grain pipelined loops    */

  Generic irr;           /* Ptr to class Irr_Globals */

  Boolean declLog2Phys;	 /* Flags marking whether these two function */
  Boolean declPhys2Log;	 /*   names have been declared for this procedure */
  
  dc_mem *mem;           /* heap memory for FortD compiler */

} Dist_Globals;


/*------------------------- function definitions ------------------------*/

EXTERN(enum RANGETYPE, dc_range_type, (int l1, int l2));
EXTERN(enum ALIGNTYPE, dc_align_type, (SNODE *sp, int dim));
EXTERN(Boolean, is_match_subscript2, (AST_INDEX node, char *str));
EXTERN(SNODE *,  findadd2, (AST_INDEX node, int flag, int level,
			    Dist_Globals *dh));

EXTERN(int,  dist_compiler,   (PedInfo ped));
EXTERN(void, dc_compile_proc, (PedInfo ped, AST_INDEX pnode, int nproc,
			       Fd_opts *fd_opts, Generic di));
EXTERN(void, dc_indices,      (Dist_Globals *dh));
EXTERN(void, dc_new_var,      (Dist_Globals *dh, AST_INDEX node,
                               enum FORM type));
EXTERN(void, init_fd_opts,    (Fd_opts *fd_opts));
EXTERN(void, dc_init,         (Dist_Globals *dh, Fd_opts* fd_opts));
EXTERN(char*, dc_get_mem,     (Dist_Globals *dh, int size));
EXTERN(void, dc_finalize,     (Dist_Globals *dh));
EXTERN(void, dc_find_loops,   (Dist_Globals *dh));

EXTERN(Dist_type, is_part, (AST_INDEX name_id, int dim, Dist_Globals *dh));

EXTERN(AST_INDEX, ast_set_logical_myproc,	(Dist_Globals *dh));
EXTERN(AST_INDEX, ast_get_logical_myproc,	(Dist_Globals *dh));
EXTERN(AST_INDEX, ast_map_logical_to_physical,	(Dist_Globals *dh, AST_INDEX logical_proc_ast));

#endif
