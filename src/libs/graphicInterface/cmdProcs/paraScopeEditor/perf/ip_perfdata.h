/* $Id: ip_perfdata.h,v 1.6 1997/03/11 14:32:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* More local declarations, typedefs, etc. for the initial info phase
 * of the performance estimator. This header file deals with data structures
 * which hold the training set data for a particular architecture.
 */

/* $Log: ip_perfdata.h,v $
/* Revision 1.6  1997/03/11 14:32:09  carr
/* newly checked in as revision 1.6
/*
 * Revision 1.6  93/12/17  14:55:36  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.5  93/11/16  16:14:20  curetonk
 * removed trailing comma from enum.
 * 
 * Revision 1.4  93/06/11  13:30:00  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.3  93/03/31  10:44:13  mcintosh
 * Fix some bugs in the macros for accessing performance
 * data. Add tags to all anonymous unions & structures
 * so as to be able to look at them in the debugger.
 * 
 * Revision 1.2  92/10/03  16:27:25  rn
 * fix include file pathnames, change EXTERN_FUNCTION to EXTERN,
 * add RCS header, and minor additional cleanup -- JMC & Don
 * 
 * Revision 1.1  92/06/23  22:44:42  mcintosh
 * Initial revision
 */

#ifndef ip_perfdata_h
#define ip_perfdata_h

/*---------------------------------------------------------------------*/

/*
 * Version number for insuring that data files are in "current" format. If a
 * data file doesn't have this version number (i.e. smaller version number or
 * missing entirely) then it will be considered an ancient or corrupted file
 * and an error will be signalled when the file is read in.
 */

#define PERF_DATAFILE_VERSION 1

/* Subdirectory of /rn where performance data files live
*/
#define PERF_DATA_FILES_HOME "perf"

/*
 * Name of environment variable checked for alternate path for perf data
 * files.
 */
#define PERF_DATA_FILES_HOME_ENV_VAR "PERF_DATA_FILES_HOME"

/* The name of the data file
*/
#define PERF_DATA_FILE_NAME "perf.data"

/* Maximum length of a line (in characters) in the performance data file.
*/
#define PERF_DATA_MAXLINE 4096

/*
 * This enumeration defines the possible architectures for which performance
 * data is available. If you modify this typedef be sure to make equivalent
 * changes in the function "perf_map_archtype_to_string();".
 */

typedef enum Perf_archtypeE {
  PE_DUMMY = 0,			/* no arch */
  PE_SEQUENT_SYMMETRY = 1,	/* Sequent Symmetry */
  PE_BBN_TC2000 = 2		/* BBN TC2000 */ /* CURRENTLY UNUSED */
  } Perf_archtype;

/*
 * Names of machines. These strings are used to locate the directory which
 * has the data file which contains the performance data for the correct
 * architecture. There is a string in this list for each item in the
 * "Perf_archtype" enumeration.
 */

#define PERF_EST_ARCH_SEQUENT_SYMMETRY   "sequent_symmetry"
#define PERF_EST_ARCH_BBN_TC2000         "bbn_tc2000"
#define PERF_EST_ARCH_DUMMY              "dummy"

/* Various keywords which may appear in the header section of the file
*/
#define PERF_ARCH_KW			"@architecture"
#define PERF_VERSION_KW			"@version"
#define PERF_MAXPARLOOPITERS_KW         "@max_parloop_iters"
#define PERF_MAXPARLOOPPROCS_KW         "@max_parloop_procs"
#define PERF_MAXPARLOOPWORKRANGE_KW     "@max_parloop_workrange"
#define PERF_MAXPARLOOPWORKINCR_KW      "@max_parloop_workincr"

/*
 * Sections in the file. Sections begin with a single keyword along on a line
 * (ex: "@computation_data") and end with the keyword "@end_section".
 */

#define PERF_END_SECTION_KW		"@end_section"
#define PERF_COMPDATA_SECTION_KW	"@computation_data_section"

/*
 * The following constants are the guesses which the performance estimator
 * uses when it has to make a guess about something in the program that can't
 * be determined during the local phase.
 */

#define PERF_GUESS_LOOP_ITERATIONS	50
#define PERF_GUESS_BRANCH_PROB          0.5

/*
 * Data on operations which depend on argument type is kept in arrays indexed
 * by the type. This macro is used to dimension these arrays.
 *
 * HACK ALERT: I'm taking advantage of the ordering of the macros in
 * $RN_SRC/include/fort/gi.h, which defines TYPE_UNKNOWN as 0, TYPE_INTEGER
 * as 1, etc., on up to TYPE_COMPLEX as 4. If this ordering were to change,
 * this way of declaring the arrays would break.
 */

#define T_OPS	(TYPE_COMPLEX+1)	/* number of different op types */

/*
 * The following data structure is the internal representation for the cost
 * of a particular operation.
 *
 * The simplest operations always take a fixed amount of time; the data stored
 * for them is simply the amount of time they take. An example of an
 * operation which falls into this category is something like an add or a
 * goto.
 *
 * More complicated operations such as function calls (where there are a
 * variable number of parameters) or "generic" functions like "max" (where
 * you can have a variable number of arguments), are represented in terms of
 * base overhead f the operation (b) plus some amount of additional overhead
 * per parameter or iteration, etc (a*x).
 */

typedef union Perf_compdataU {
  double val;			/* execution time in seconds for simple ops */
  struct Perf_compdataPvalS {
    double base;		/* base time for this operation (b) */
    double incr;		/* additional increment (a*x) */
  } pval;			/* data for "a*x + b" operations */
} Perf_compdata;

/*
 * Parallel loop cost data is something of a special case. The questions
 * that we want to ask the training set are: 1) if we run the loop in
 * parallel using some number of processors, how fast will it run, and 2)
 * how many processors should we use (assuming that we want good speedup
 * but don't want to arbitrarily use all the processors). The assumption
 * is that the longer the loop would take serially, the more processors we
 * want to throw at it to get good speedup.
 * 
 * Let L be a parallel loop. Let CS be the cost of executing a single
 * iteration of the loop serially. Let NI be the number of iterations of
 * the loop. The training set data can be viewed as a CS x NI table, where
 * each element is a structure containing the best number of processors
 * and the best execution time. For example, suppose I have a loop with 2
 * iterations and a 2 millisecond loop body. I look up the table entry
 * (2,2) <this abstracts a few details of the lookup> and I get a
 * structure back. The structure tells me whether I should run the loop in
 * parallel and, if so, how long it will take.
 */

typedef struct Perf_parloopdataS {
  double bodycost;		/* minimal serial cost of loop body */
  double parloopcost;		/* cost of running loop in parallel */
  int nprocs;                   /* best number of processors */
} Perf_parloopdata;

/*
 * This is the "top level" data structure which holds performance data for a
 * particular architecture (identified by the 'arch_id' field). The "comp"
 * field holds all computation-related data; additional fields can be added
 * to this structure for things like communication overhead (for distributed
 * memory machines, perhaps).
 *
 * We distinguish between computational operations which are typed and those
 * which are untyped. An addition, for example, depends on the type of its
 * arguments (double precision adds may take longer than integer adds). Typed
 * operations are stored in the array "typed", which is indexed by the
 * argument type (TYPE_INTEGER, TYPE_REAL, etc).
 *
 * Data for the "generic" and "intrinsic" functions are stored in their own
 * tables. These tables are indexed first by the type of the argument type,
 * and then by the function itelf (in alphabetical order-- see the tables in
 * FortTree/builtins.c.
 *
 * The field "parloop" will be filled in with a dynamically allocated
 * array (see the parallel loop cost macro defined below
 * for more info). The length of the the array will be the max # of
 * processors for the machine (+1).
 */

typedef struct Perf_dataS {
  Perf_archtype arch_id;		/* ID of target architecture */
  struct Perf_dataScompS {
    Perf_compdata *typed[T_OPS];	/* typed operations */
    Perf_compdata *untyped;		/* untyped operations */
    Perf_compdata *intrinsic;		/* intrinsic functions */
    Perf_compdata *generic[T_OPS];      /* generic functions */
    Perf_parloopdata **parloop;         /* vector of vectors of parloop data */
  } comp;				/* computation data */
  int max_parloop_iters;
  int max_parloop_procs;
  int max_parloop_workrange;
  int max_parloop_workincr;
} Perf_data;

/* Top level routines to read data, then free it */
EXTERN(Perf_data *, perf_read_data, (Perf_archtype arch));
EXTERN(void, free_Perf_data, (Perf_data *freeit));

/*
 * The following macros (with prefix _PE) are private to this file; they are
 * used only to build other macros.
 */

/* Untyped simple operations (single data value)
*/
#define _PE_SU_GOTO	0	/* goto statement */
#define _PE_SU_LOGOP	1	/* logical operation (and, or, ..) */
#define _PE_SU_CMP	2	/* compare (.lt., .gt., ...) */

#define _PE_SU          2       /* last simple, uptyped operation */
#define _PE_NUM_SU      3       /* number of simple, untyped operations */

/* Untyped parameterized operations (base + n * increment)
*/
#define _PE_PU_PCALL	(_PE_SU+1)	/* proc call with N params */
#define _PE_PU_FCALL	(_PE_SU+2)	/* func call with N params */
#define _PE_PU_DOLOOP	(_PE_SU+3)	/* do-loop overhead (per iteration) */

#define _PE_NUM_PU      3               /* number of untyped parameterized */

/* Typed simple operations (single data value)
*/

#define _PE_SU_ASSIGN	0	/* simple assignment */
#define _PE_ST_ADD	1	/* add */
#define _PE_ST_SUB	2	/* subtract */
#define _PE_ST_MULT	3	/* multiply */
#define _PE_ST_DIV	4	/* divide */
#define _PE_ST_EXP	5	/* exponentiation */

#define _PE_ST		5
#define _PE_NUM_ST      6       /* # of typed, simple operations */

/* Typed parameterized operations (base + n * increment)
*/

#define _PE_PT_ARR	(_PE_ST+1)	/* subscripted array reference */

#define _PE_NUM_PT	1		/* number of typed/param ops */

/*----------------------------------------------------------------------*/
/* Public macros
*/

#define PERF_EST_N_UNTYPED_SIMPLE _PE_NUM_SU	/* untyped, simple */
#define PERF_EST_N_UNTYPED_PARAM  _PE_NUM_PU	/* untyped, parameterized */
#define PERF_EST_N_UNTYPED   (_PE_NUM_SU+_PE_NUM_PU)    /* total untyped */
#define PERF_EST_N_TYPED_SIMPLE   _PE_NUM_ST	/* typed, simple */
#define PERF_EST_N_TYPED_PARAM    _PE_NUM_PT	/* typed, parameterized */
#define PERF_EST_N_TYPED     (_PE_NUM_ST+_PE_NUM_PT)    /* total typed */

/*
 * Macros to get the cost of individual operations. The "ptr"
 * argument is a pointer to a variable of type "Perf_data";
 * "n" is assumed to be an integer. All macros return a value
 * of type double.
 */

#define PE_COST_IF(ptr)		(ptr->comp.untyped[_PE_SU_IF].val)
#define PE_COST_GOTO(ptr)	(ptr->comp.untyped[_PE_SU_GOTO].val)
#define PE_COST_LOGOP(ptr)	(ptr->comp.untyped[_PE_SU_LOGOP].val)
#define PE_COST_CMP(ptr)	(ptr->comp.untyped[_PE_SU_CMP].val)
#define PE_COST_ASSIGN(ptr)	(ptr->comp.untyped[_PE_SU_ASSIGN].val)

#define PE_COST_PCALL(ptr,n) (ptr->comp.untyped[_PE_PU_PCALL].pval.base + \
			      (n * ptr->comp.untyped[_PE_PU_PCALL].pval.incr))
#define PE_COST_FCALL(ptr,n) (ptr->comp.untyped[_PE_PU_FCALL].pval.base + \
			      (n * ptr->comp.untyped[_PE_PU_FCALL].pval.incr))
#define PE_COST_DOLOOP(ptr,n) (ptr->comp.untyped[_PE_PU_DOLOOP].pval.base + \
			      (n * ptr->comp.untyped[_PE_PU_DOLOOP].pval.incr))

#define PE_COST_ADD(ptr,typ)	((ptr->comp.typed[typ])[_PE_ST_ADD].val)
#define PE_COST_SUB(ptr,typ)	((ptr->comp.typed[typ])[_PE_ST_SUB].val)
#define PE_COST_MULT(ptr,typ)	((ptr->comp.typed[typ])[_PE_ST_MULT].val)
#define PE_COST_DIV(ptr,typ)	((ptr->comp.typed[typ])[_PE_ST_DIV].val)
#define PE_COST_EXP(ptr,typ)	((ptr->comp.typed[typ])[_PE_ST_EXP].val)

#define PE_COST_ARR(ptr,ndim,typ) \
                    ((ptr->comp.typed[typ])[_PE_PT_ARR].pval.base + \
                     (ndim * (ptr->comp.typed[typ])[_PE_PT_ARR].pval.incr))

/*
 * Macros for cost of intrinsic and generic functions. 
 */

#define PE_COST_INTRINSIC(ptr,i_name) \
  perf_get_intrinsic_cost(ptr,i_name)

#define PE_COST_GENERIC(ptr,g_name,typ,nargs) \
  perf_get_generic_cost(ptr,g_name,typ,nargs)

EXTERN(double, perf_get_intrinsic_cost,
		(Perf_data *pdata, char *i_name));
EXTERN(double, perf_get_generic_cost,
		(Perf_data *pdata, char *g_name,
		 int typ, int nargs));

/*
 * Macro for getting the cost of a parallel loop. In this case, we can't do a
 * straight table lookup, so call a subroutine instead. The first argument is
 * a pointer to the Perf_data structure. The second is the serial cost of
 * executing the loop (i.e. the # of iterations times the cost of executing
 * the body). The third argument is a pointer a "double" in which will be
 * returned the execution time of the loop when run in parallel. The fourth
 * parameter is a pointer to an integer in which will be returned the number
 * of processors used for this parallel loop.
 */

#define PE_COST_PARLOOP(ptr,serialcost,niterations,parallelcostptr,nprocptr) \
	perf_get_parallel_loop_cost(ptr,serialcost,niterations,\
				    parallelcostptr,nprocptr)

/* Routine to calculate parallel cost */
EXTERN(void, perf_get_parallel_loop_cost,
		(Perf_data *perfdata, double serial_cost,
		 int num_iterations, double *returned_parallel_cost,
		 int *returned_nprocs));

/*----------------------------end of file------------------------------------*/

#endif /* ip_perfdata_h */
