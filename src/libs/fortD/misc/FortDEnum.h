/* $Id: FortDEnum.h,v 1.7 1997/03/11 14:28:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-------------------------------------------------------------

    FortDEnum.h    Enum types for Fortran D compiler & interface

*/
#ifndef _FortDEnum_
#define _FortDEnum_

/**************************************************************/
/* description of the distribution type of an array dimension */
/**************************************************************/

typedef enum Dist_type
{
  FD_DIST_DC_NONE = 1,             /* not distributed           */
  FD_DIST_LOCAL = 1,               /* not distributed           */
  FD_DIST_BLOCK = 2,               /* BLOCK distribution        */
  FD_DIST_CYCLIC = 3,              /* CYCLIC distribution       */
  FD_DIST_BLOCK_CYCLIC = 4,        /* BLOCK_CYCLIC distribution */
  FD_DIST_USER = 5                 /* user-defined distribution */
} Dist_type;

/**************************************************************/
/* description of the loop affecting comm caused by reference */
/**************************************************************/

typedef enum Mesg_type
{
  FD_MESG_INDEP,        /* loop-independent message                     */
  FD_MESG_CARRIED_ALL,  /* loop-carried message on non-partitioned loop */
  FD_MESG_CARRIED_PART, /* loop-carried message on partitioned loop     */
  FD_MESG_REDUC,        /* reduction messages (for queries only)        */
  FD_MESG_ALL           /* all types of messages (for queries only)     */
} Mesg_type;

/****************************************************************/
/* description of the type of communication caused by reference */
/****************************************************************/

typedef enum Comm_type
{
  FD_COMM_UNKNOWN,      /* unknown comm type */
  FD_COMM_NONE,         /* no communication */
  FD_COMM_SEND_RECV,    /* single send/receive */
  FD_COMM_SHIFT,        /* shift by multiple procs */
  FD_COMM_BCAST,        /* broadcast/spread by proc */
  FD_COMM_REDUCE,       /* reduce by procs */
  FD_COMM_GATHER,       /* gather by procs */
  FD_COMM_TRANSPOSE,    /* transpose between procs */
  FD_COMM_INSPECT,      /* communication taken care of by inspector */
  FD_COMM_RUNTIME       /* communication determined only at runtime */
} Comm_type;
 
/**********************************************************/
/* description of the type of reduction operation         */
/**********************************************************/

typedef enum Reduc_type
{
  FD_REDUC_NONE =  0,
  FD_REDUC_PLUS =  1,
  FD_REDUC_MINUS = 2,
  FD_REDUC_TIMES = 3,
  FD_REDUC_DIV =   4,
  FD_REDUC_MIN =   5,
  FD_REDUC_MAX =   6,
  FD_REDUC_OR  =   7,
  FD_REDUC_AND =   8,
  FD_REDUC_XOR =   9,
  FD_REDUC_MIN_LOC = 10,
  FD_REDUC_MAX_LOC = 11
} Reduc_type;

/**************************************************************/
/* description of the type of statement execution             */
/**************************************************************/

typedef enum Stmt_type
{
  FD_STMT_PARALLEL, 
  FD_STMT_REPLICATED, 
  FD_STMT_ONE_PROC, 
  FD_STMT_SUM_REDUCT, 
  FD_STMT_PROD_REDUCT,
  FD_STMT_MAX_REDUCT, 
  FD_STMT_MIN_REDUCT, 
  FD_STMT_MAXLOC_REDUCT, 
  FD_STMT_MINLOC_REDUCT
} Stmt_type;


/**************************************************************/
/* description of the type of execution of loop iterations    */
/**************************************************************/

typedef enum Loop_type
{
  FD_LOOP_REPLICATED,    /* iterations executed by all procs              */
  FD_LOOP_PARALLEL,      /* iterations executed in parallel               */
  FD_LOOP_PIPELINED,     /* iterations executed in pipelined fashion      */
  FD_LOOP_SEQUENTIAL,    /* pipelined iterations executed sequentially    */
  FD_LOOP_ONEPROC        /* iterations executed by only one processor     */
} Loop_type;


typedef enum Color_type 
{
  FD_RED,             /* DEditor: Bad         */
  FD_YELLOW,          /* DEditor: Not too bad */
  FD_GREEN,           /* DEditor: Okay        */
  FD_BLACK            /* DEditor: Good        */
} Color_type;


/**************************************************************/
/* Fortran D compiler target architectures                    */
/**************************************************************/

typedef enum Fd_Arch_type
{
  FD_NX,                 /* Intel iPSC/860, Delta, Paragon     */
  FD_CMMD,               /* Thinking Machines CM-5             */
  FD_PVM,                /* Oak Ridge Parallel Virtual Machine */
  FD_EXPRESS,            /* Parasoft Express                   */
  FD_KSR,                /* Kendall Square Research            */
  FD_SGI,                /* Silicon Graphics Multiprocessor    */
  FD_DASH                /* Stanford Dash Multiprocessor       */
} Fd_Arch_type;

#endif
