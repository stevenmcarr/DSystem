/* $Id: perf.h,v 1.10 2001/09/17 00:32:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
  perf.h -- declarations for the performance estimation module.

  >> The software in this file is public domain. You may copy, modify and 
  >> use it as you wish, provided you cite the author for the original source.
  >> Remember that anything free comes with no guarantees. 
  >> Use it at your own risk. 

  Author: Vas, July 1990.

  Modification History:
  
 *****************************************************************************/

#ifndef perf_h
#define perf_h

#ifndef dp_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#endif

/*=================  NOTE: machine-dependent binding ================*/
#define  PACKET_SIZE   1024   /* pkt size in bytes of target machine */


/* internal debugging switch */
#define  DEBUG 0
#define  debug     if (DEBUG) printf

#define  NOT_EXPRESS   0

/* arithmetic and control operations */
#define ARITH_CNTL      0
#define NUM_ARITH_CNTL  15

/* list of arithmetic and control operations analyzed */
#define T_DOLOOP          0
#define T_IFTHEN          1
#define T_GOTO            2
#define T_CALL            3
#define T_FUNCTION        4
#define T_ARRAYREF        5
#define T_INTADD          6
#define T_INTMINUS        7
#define T_INTTIMES        8
#define T_INTDIVIDE       9
#define T_FLOATADD        10
#define T_FLOATMINUS      11
#define T_FLOATTIMES      12
#define T_FLOATDIVIDE     13
#define T_EXPONENT        14
#define T_MISC            8  /* all others */

/* list of communication routines analyzed */
#define iSR    1
#define vSR    2
#define EXCH1  3
#define BCAST  4
#define COMBN  5

/* data stride */
#define UNIT      1
#define NON_UNIT  2

/* default lower and upper bounds for IF branch probability */
#define DEFAULT_BRANCH_PROB_LB  0.25
#define DEFAULT_BRANCH_PROB_UB  0.75

#define MINUS_INFTY -9999
  
/* boundaries of the selected program segment */
extern AST_INDEX   first_stmt;
extern AST_INDEX   last_stmt;

/* structure to keep communication data for each packet (an element of the LIST)  */
typedef struct CData {
  int          xrange[2];  /* range of x axis data (msg length in bytes) */
  float        a;     /* equation of a line segment of graph is given by          */
  float        b;     /* y = a + bx, where a and b are computed using chi**2 test */
  struct CData *next;
} CommData;

/* communication data information (an element of the ARRAY) */
typedef struct CInfo {
  float        StartCost;     /* comm startup cost */
  float        pktization;  /* cost of msg packetization */
  CommData     *commdata;   /* ptr to the LIST of data for each of the packets */ 
} CommInfo;

/* structure to keep training set data as a linked list */
typedef struct TData {
  int          nprocs;  /* number of procs */
  int          dstride; /* stride of data in local memory */
  int          numcomm; /* number of communication utilities tested */
  int          npoints; /* num of data points per comm utility */
  float        arithdata[NUM_ARITH_CNTL]; /* arithmetic and control data */
  CommInfo     *comminfo;  /* ARRAY of info about communication utilities */
  struct TData *next;
} TrainData;

struct Prmtr;
typedef struct Prmtr Parm;

/* global var to keep pointer to training set data read by perf_init() */
extern TrainData   *Train_Data_List;

/* global var to keep number of processors used */
extern int   Num_Procs;

/* global Dependence pointer */
extern PedInfo  Global_Dep_Ptr;

/* global ast index of PROGRAM statement */
extern AST_INDEX  Program_Root;

EXTERN (TrainData*, perf_init, (void));
EXTERN (char*, get_ctype, (int ctype));
EXTERN (void, Numprocs_initialize, (AST_INDEX root));
EXTERN (void, chi_squared_fit, (char *databuf, int npoints, CommInfo *commptr));
EXTERN (int, perf_estimate, (AST_INDEX fstmt, AST_INDEX lstmt, float *ccost_lb, 
                             float *ecost_lb, float *ccost_ub, float *ecost_ub));
EXTERN(void, get_call_info, (AST_INDEX stmt, char *name, AST_INDEX *parmlist,
                             int *numparms, AST_INDEX *body));
EXTERN(void, get_function_info, (AST_INDEX stmt, char *name, AST_INDEX *parmlist,
                                 int *numparms, AST_INDEX *body));
EXTERN (void, get_assignment_info, (AST_INDEX stmt, AST_INDEX *left, 
                                    AST_INDEX *right));
EXTERN (int, get_nesting_level, (AST_INDEX stmt));
EXTERN (void, resolve_symbols, (AST_INDEX stmt, int num_symbols, AST_INDEX *parms,
                                int *values));
EXTERN (void, resolve_symbols_dialog, (AST_INDEX stmt, int num_symbols, char *symstr,
                                      int *sindex, int *values));
EXTERN (void, get_branch_probability, (AST_INDEX stmt, float *prob_lb, float *prob_ub));
EXTERN (void, process_parameter_stmt, (AST_INDEX stmt));

EXTERN(int, check_if_int, (AST_INDEX node));
EXTERN(int, check_if_Express, (AST_INDEX stmt));
EXTERN(void, get_msg_info, (int commtype, AST_INDEX stmt, int *msg_size, 
                            int *data_stride));
EXTERN(float, time_comm, (int commtype, int msg_size, int dstride, float *StartCost,
                          float *pktization));
EXTERN(void, correct_for_overlap, (int rc, AST_INDEX node, float *ctime));
EXTERN(Parm*, push, (Parm *item));
EXTERN(Parm*, pop, (Parm **item));

/* symbol table initialization. Not needed once Seema's stuff
   is integrated. Switch to the symtab routines in that code --Vas */
// extern symtab_initialize();


#endif
