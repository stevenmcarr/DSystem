/* $Id: dep_dt.h,v 1.23 1997/03/11 14:35:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*-----------------------------------------------------------------------

	dt.h		Dependence Test Module Header File

	History
	~~~~~~~
	15 Feb 90  cwt  Created
	22 May 91  cwt  Add support for REF
*/

/* RCS Revision history:
 * $Log: dep_dt.h,v $
 * Revision 1.23  1997/03/11 14:35:50  carr
 * newly checked in as revision 1.23
 *
 * Revision 1.23  93/12/22  09:46:25  curetonk
 * added an EXTERN prototype for dg_ast_dim.
 * 
 * Revision 1.21  93/09/30  14:23:38  curetonk
 * changes to make dep_dt.h ANSI-C compliant
 * 
 * Revision 1.20  93/09/17  14:48:14  nenad
 * Fixed the prototype for dt_test.
 * 
 * Revision 1.19  93/08/28  10:09:03  nenad
 * Added new fields to Dg_ref_info struture and updated some prototypes.
 * 
 * Revision 1.18  93/07/31  10:35:32  nenad
 * Extended Subs_data and Loop_data structure to support symbolic analysis.
 * Added new #define SUBS_SYM_* for more precise classification of subscripts.
 * 
 * Revision 1.17  93/06/21  15:36:25  nenad
 * Changed prototype for dt_analyze() which now takes cfgInst as an
 * additional argument.
 * 
 * Revision 1.16  93/06/04  13:19:53  mpal
 * Update prototype for dt_test(), add CfgInstance for symbolic info.
 * mpal: 930604
 * 
 * Revision 1.15  92/12/08  15:48:37  carr
 * dg_header.h and el_header.h
 * 
 * Revision 1.14  92/11/20  15:08:49  joel
 * *** empty log message ***
 * 
 * Revision 1.12  92/10/01  16:47:00  mpal
 * This is the public interface to dependence testing.
 * 
 * Revision 1.11  92/07/02  13:38:53  mpal
 * change prototypes for dt_init() and dt_update_loopref()
 * 
 * Revision 1.10  92/06/23  16:44:29  seema
 * separated expr structures
 * 
 * Revision 1.9  92/06/04  00:23:50  mpal
 * Abstraction of the dependence graph from the PedInfo structure
 * Filled in parameter lists for EXTERN macros
 * 
 * Revision 1.8  92/05/29  18:02:01  tseng
 * clean up old comments
 * 
 * Revision 1.7  92/05/13  14:20:13  mpal
 * add the "#ifdef  dt_h" wrapper to ensure only one copy is loaded.
 * 
 * Revision 1.6  91/10/07  10:54:48  tseng
 * partial support for symbolic loop bounds - tseng
 * 
 * Revision 1.5  91/09/08  19:17:57  rn
 *  9/8/91 kats - corrected external declaration format
 * 
 * Revision 1.4  91/08/13  10:31:36  mcintosh
 * Changed defs of DDATA consts to work around a compiler problem -NM
 * 
 */

/*----------------------------- defines ----------------------------*/

#ifndef	dt_h
#define	dt_h

#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	expr_h
#include <libs/frontEnd/include/expr.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif
#ifndef	cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif


#ifndef MAXINT
#define MAXINT  (0x7fffffff)
#endif

#ifndef MININT
#define MININT  (0x80000000)
#endif

/*--------------------------*/
/* types for Edge.dt_type	*/

/* this field characterizes the entire data dependence */

#define DT_UNKNOWN  0			/* unknown dependence type   */
#define DT_LBITS	5		/* # of bits used for levels	 */
#define DT_LVL		0x001f		/* mask for deepest loop level	 */
#define DT_CLVL		0x03e0		/* mask for common loop level	 */
#define DT_NOPROVE	0x0400		/* bit for not proven deps	 */
#define DT_DIS		0x0800		/* bit for direction vector	 */
#define DT_EQ		0x1000		/* bit for loop indep possible	 */
#define DT_ALL_EQ	0x2000		/* bit for only loop indep	 */
#define DT_SCALAR	0x4000		/* bit for scalar deps		 */
#define DT_NONLIN	0x8000		/* bit for nonlinear subs	 */
#define DT_NONE		0x10000		/* no dependence		 */

/*--------------------------*/
/* codes for Edge.dt_data   */

/* this field characterizes the data dependence at each loop level */

/* Edge->dt_data[N-1] -> dep info for loop at depth N		*/

/* info is either 1) dep distance 				*/
/*                2) dep direction (if < DDATA_BASE)		*/

#define DDATA_LT    ((int) MININT + 1)	/* less than			 */
#define DDATA_GT    ((int) MININT + 2)	/* greater than			 */
#define DDATA_LE    ((int) MININT + 3)	/* less than or equal		 */
#define DDATA_GE    ((int) MININT + 4)	/* greater than	or equal	 */
#define DDATA_NE    ((int) MININT + 5)	/* not equal			 */
#define DDATA_ANY   ((int) MININT + 6)	/* any 				 */
#define DDATA_ERROR ((int) MININT + 7)	/* error 			 */
#define DDATA_BASE  ((int) MININT + 8) 	/* DDATA codes are < BASE	 */

/*---------------------------*/
/* codes for Subs_data.stype */

/* this field characterizes each subscript expression */

								/* N = 0-20 -> SIV sub at loop N */
#define SUBS_SIV     20			/* single index var subscripts   */
#define SUBS_MIV     21			/* multiple index var subscripts */
#define SUBS_ZIV     22			/* zero index var subscripts  	 */
#define SUBS_RDIV    23			/* restricted double index var subs */
#define SUBS_EMPTY   24			/* subscript analyzed prev	 	 */

#define SUBS_SYM     29 		/* too complex/symbolic */
#define SUBS_SYM_SIV_FIRST 30 	/* N >= 30 -> SYM_SIV at loop N-30 */
#define SUBS_SYM_SIV_LAST  50 	/* SYM_SIV = SIV w/ loop invariant symbolic */
#define SUBS_SYM_MIV 51 		/* MIV with loop invariant symbolic */
#define SUBS_SYM_ZIV 52 		/* ZIV with loop invariant symbolic */

/*----------------------------------------------*/
/* information for one loop */

typedef struct		   /* data for single loop 			 */
{
	Expr lo;                 /* loop lower bound */
	Expr up;                 /* loop upper bound */
	Expr step;               /* loop step        */

	int lo_val;        		 /* value number for lower bound */
	int up_val;              /* value number for upper bound */

	int *lo_vec;             /* vector of coeffs in trapezoidal loop nest */
	int *up_vec;             /* vector of coeffs in trapezoidal loop nest */

    AST_INDEX loop_index;    /* AST index of loop       */
    char *ivar;              /* str of index variable   */
    Boolean	rev;             /* loop was reversed?      */

} Loop_data;

/*----------------------------------------------*/
/* information for loop nest */

typedef struct			/* list of loops			 */
{
	Loop_data       loops[MAXLOOP];		/* one for each loop	 */
	int             level;			/* number of loop nests	 */
} Loop_list;

/*----------------------------------------------*/
/* information for individual subscript */

typedef struct			/* single subscript data	 */
{
	int            stype;			/* type = ZIV, SIV, MIV  */
	int            constant;		/* constant portion	 */
	int            coeffs[MAXLOOP];		/* list of coefficients	 */
	int      	   symbolic_constant; 	/* value number of symbolic constant */
	AST_INDEX      sym;			/* symbolic expression	 */
} Subs_data;

/*---------------------------------------------------*/
/* information for all subscripts of array reference */

typedef struct 		/* list of subscripts		 */
{
	Subs_data	subs[MAXDIM];	/* one for each dimension */
	int		dims;		/* number of dimensions	 */
	Loop_list	*loop_nest;	/* ptr to enclosing Loop_list */
} Subs_list;

/* --------------------------------------------------------------------	*/
/* ---------- structure and allowed values for Dg_ref_info ------------	*/

typedef struct dg_ref_info
{
	AST_INDEX node;		/* AST of reference	    */
	int 	  def;		/* use, def, or guarded def */
	char 	 *sym;		/* symbols for ref	    */
	AST_INDEX stmt;         /* AST of statement         */
	AST_INDEX loop;         /* AST of enclosing loop    */
	int       index;        /* reference SymTable index */
	int       dims;         /* number of dimensions     */
	int       leader;       /* SymTable index of equiv. class leader */
	int       offset;       /* offset in the equivalence class */
	int       size;         /* size of the reference    */
	int       elem_size;    /* size of the type in bytes */
} Dg_ref_info;

				/* types for Dg_ref_info.def		*/
				/* ------------------------------------	*/
#define T_USE	           1	/* reference is an use			*/
#define T_DEF	           2	/* reference is an unguarded def	*/
#define T_GUARDED          4	/* reference is guarded (defs only) 	*/

#define T_IP_WHOLE_ARRAY   8	/* from interprocedural mod/ref (assume callee
				 * references the actual as an array)
				 */
#define T_IP_WHOLE_COMMON  16	/* from interprocedural mod/ref reference to
                                 * unknown common
				 */

#define T_IP_SCALAR        32	/* from interprocedural mod/ref  + ip formal 
				 * array information: callee references the
				 * argument passed as a scalar
				 */

#define T_IP_GLOBAL        64   /* global side-effect of a call */
#define T_IP_CONSERVATIVE  128  /* conservative inference at a callsite */

/*	The following functions are defined in	dep/dt/dt_build.c	*/

/* Get the Dg_ref_info structures for each reference in loop, and count	*/
EXTERN(void, dg_ref_info_and_count, 
		( FortTree ft, AST_INDEX loop, int * count, Dg_ref_info ** refArray ) );

/* Free the Dg_ref_info structures allocated by dg_ref_info_and_count() */
EXTERN(void, dg_ref_info_free, ( Dg_ref_info * refArray ) );


/*	Field Access Functions for the Dg_ref_info structure		*/
EXTERN(AST_INDEX, dg_ref_info_node, ( Dg_ref_info * ref_info ) );

EXTERN(char *,  dg_ref_info_sym, ( Dg_ref_info * ref_info ) );

EXTERN(int, dg_ref_info_def, ( Dg_ref_info * ref_info ) );

EXTERN(int, dg_ast_dim, ( AST_INDEX node ) );



/*----------------------------- macros ----------------------------*/

/*----------------------------------*/
/* macros to access Dt_info fields  */
/*----------------------------------*/

#define gen_get_dt_LVL(dt)		((dt)->dt_type & DT_LVL)
#define gen_put_dt_LVL(dt,lvl)		((dt)->dt_type |= ((lvl) & DT_LVL))

#define gen_get_dt_CLVL(dt)		(((dt)->dt_type & DT_CLVL) >> DT_LBITS)
#define gen_put_dt_CLVL(dt,lvl)		((dt)->dt_type |= (((lvl) << DT_LBITS) & DT_CLVL))

#define gen_is_dt_NOPROVE(dt)		((dt)->dt_type & DT_NOPROVE)
#define gen_is_dt_DIS(dt)		((dt)->dt_type & DT_DIS)
#define gen_is_dt_EQ(dt)		((dt)->dt_type & DT_EQ)
#define gen_is_dt_ALL_EQ(dt)		((dt)->dt_type & DT_ALL_EQ)
#define gen_is_dt_SCALAR(dt)		((dt)->dt_type & DT_SCALAR)
#define gen_is_dt_NONLIN(dt)		((dt)->dt_type & DT_NONLIN)

#define gen_get_dt_DIS(dt, lvl)		((dt)->dt_data[(lvl)-1])
#define gen_put_dt_DIS(dt,lvl,val)	((dt)->dt_data[(lvl)-1] = (val))
#define gen_is_dt_DIR(dis)		((dis) < DDATA_BASE)

/*----------------------------- declarations ----------------------------*/

/*---------------*/
/* global decls  */
/*---------------*/

/* dependence test entry points in dep/dt/dt.c				*/

/* called to free memory used by DT_info structure	*/
EXTERN(void, dt_free, ( DT_info * dt ) );

/* called once to init deps	*/
EXTERN(DT_info *,	 dt_init, ( AST_INDEX root, SideInfo * infoPtr, 
								CfgInfo cfgModule ));

/* incr update of deps		*/
EXTERN(void, dt_update, ( DG_Instance * dg, DT_info * dt,
				 SideInfo * infoPtr, AST_INDEX root ));

/* update of deps after reading dep graph */
EXTERN(void, dt_update_tests, ( DG_Instance *dg, DT_info *dt, 
				SideInfo *infoPtr, AST_INDEX root, 
				CfgInfo cfgModule ));

/* incr update of loop/ref  */
EXTERN(void, dt_update_loopref, 
		( DT_info * dt, SideInfo * infoPtr, AST_INDEX root ));

/* get to subscript AST		*/
EXTERN(AST_INDEX, dt_ast_sub, ( AST_INDEX node ));

/* get to statement AST		*/
EXTERN(AST_INDEX, dt_ast_stmt, ( AST_INDEX node ));

/* get to loop AST		*/
EXTERN(AST_INDEX, dt_ast_loop, ( AST_INDEX node ));

/* apply dep test to ref pair   */
/* if cfgInst==NULL, no symbolic analysis is needed or used	*/
EXTERN(void, dt_test, ( DT_info * dt, SideInfo * infoPtr, 
			CfgInstance cfgInst, DG_Edge * Edge,
			AST_INDEX loop1, AST_INDEX loop2,
		        Boolean imprecise ));

/* set interchange bits of edge */
EXTERN(void, dt_set_intchg, ( DG_Edge * Eptr, int level ));

/* Analyze and convert one dimension of subscript */
EXTERN(Boolean, dt_sub, 
		( AST_INDEX node, Subs_data * sdata, 
		 Loop_list * ldata, Boolean plus ) );


/* dependence test entry points in dep/dt/dt_analyze.c				*/

EXTERN(void, dt_analyze, ( DT_info * dt, DG_Edge * Edge, CfgInstance cfgInst, 
			   Subs_list * subs1, Subs_list * subs2,
			   Loop_list * loops1, Loop_list * loops2,
			   int clevel, Boolean indep ));
/* analyze deps between two ref */


/* dependence test entry points in dep/dt/dt_build.c				*/

/* Builds DG and LI for entire program given DG_Instance, ...		*/
/* Requires that the DG and LI structures already exist.		*/
EXTERN( void,	dg_build, ( AST_INDEX	 root,
	FortTree	 ft, DG_Instance	*dg, SideInfo	*infoPtr,
	DT_info		*dt, LI_Instance	*li, Generic	 pgm_callgraph, 
	CfgInfo cfgModule) );

/* Rebuild DG and LI for selected subtree			*/
EXTERN( void,	dg_update, ( AST_INDEX	 root,
	FortTree	 ft, DG_Instance	*dg, SideInfo	*infoPtr,
	DT_info		*dt, LI_Instance	*li) );


/* copy dependence info	        */
EXTERN(void, dt_copy_info, ( DT_info * dt, DG_Edge * from, DG_Edge * to ));

/* create dt str from dep info	*/
EXTERN(void, dt_info_str, ( DT_info * dt, DG_Edge * Edge ));


/* dependence test entry points in dep/dt/dt_info.c				*/

EXTERN(Loop_list *,	 dt_alloc_loop, (DT_info * dt));
EXTERN(Subs_list *,	 dt_alloc_ref, (DT_info * dt));

/* local string allocation, persistant during life of DT_Info	*/
EXTERN(char *,  dt_ssave, (char * str, DT_info * dt));

#endif

