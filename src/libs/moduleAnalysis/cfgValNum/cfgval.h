/* $Id: cfgval.h,v 1.9 1997/03/11 14:35:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef cfgval_h
#define cfgval_h

#ifndef ssa_h
#include <libs/moduleAnalysis/ssa/ssa.h>
#endif

/* #ifndef forttypes_h */
/* #include <fort/forttypes.h> */
/* #endif */

#ifndef val_h
#include <libs/moduleAnalysis/valNum/val.h>
#endif

/*
 *  cfgval.h
 *
 *  Public declarations for value numbering on the CFG/SSA.
 */

EXTERN(void, cfgval_Open,		(CfgInfo globalInfo, 
					 Boolean useIpVals) );
    /*
     *  Given the CfgInfo for a module, initialize the corresponding
     *	data structures for value numbering the contained subprograms.
     */
EXTERN(void, cfgval_Close,		(CfgInfo globalInfo) );
    /*	
     *  Free all value numbering data structures associated with this
     *	module (at least, through this CfgInfo).
     */
EXTERN(void, cfgval_Dump,		(Generic junk, CfgInstance cfg) );
    /*	
     *  Print all value table entries for the subprogram corresponding to
     *	the CfgInstance.
     */
EXTERN(void, cfgval_Save,		(CfgInstance cfg, Context) );
    /*
     *  Print value number information to database/interprocedural file.
     */
EXTERN(void, cfgval_Restore, (CfgInstance cfg, Context m_context));
    /*
     *  Read value number information from database/interprocedural file.
     */

EXTERN(void, cfgval_build_table, 	(CfgInstance cfg));
    /*
     *  Build value numbers for every expression in the procedure.
     *  This is only needed for testing purposes -- values are built
     *  on demand.
     */
EXTERN(void, cfgval_build_ip,	 	(CfgInstance cfg));
    /*
     *  Build value numbers for the interprocedurally interesting 
     *  values -- tests (for ip dead code elimination), subscripts
     *  (for ip array section analysis), and passed/returned variables
     *  (for ip symbolic analysis).
     */

EXTERN(int, cfgval_table_max, 		(CfgInstance cfg));
    /*
     *  Returns integer guaranteed to be > any valid value number
     *  (but only as long as you don't ask for any more value numbers)
     */

EXTERN(ValNumber, cfgval_get_val,      (CfgInstance cfg, AST_INDEX node));
    /*
     *	Builds a value number on demand for a particular AST node.
     *  Should be an expression, variable ref, or definition.
     */
EXTERN(Boolean,	  cfgval_get_is_const, (CfgInstance cfg, AST_INDEX astId));
    /*
     *	Returns true if the AST def/exp has a known constant 
     *	value, false otherwise.  Other routines are guaranteed 
     *	to produce useful constants only if this one returns true.
     */
EXTERN(long,	  cfgval_get_int,      (CfgInstance cfg, AST_INDEX astId));
    /*
     *  Returns the integer constant value -- currently 
     *	meaningless if the value is not actually constant.
     */

EXTERN(ValNumber, cfgval_build,      (CfgInstance cfg, SsaNodeId ssaId));
    /*
     *  Value number for SSA node -- should represent variable ref
     *  or expression.
     */
EXTERN(ValNumber, cfgval_chase_edge, (CfgInstance cfg, SsaEdgeId edgeId));
    /*
     *  Value number for source of SSA edge.
     */

EXTERN(Values, cfgval_get_values,       (CfgInstance cfg));
    /*
     *  Value table for a CFG, for passing to pure VAL functions
     */
EXTERN(fst_index_t, cfgval_get_ftsym,	(CfgInstance cfg, ValNumber valNum));

/*
 *	Data structures for interface between dependence testing 
 *	and value numbering.				paco April 93
 *
 *----------------------------------------------
 * information for one loop 
 */
typedef struct loopvalsstruct	/* data for single loop	*/
{
    ValNumber ival;		/* Value number for index variable */
    ValNumber lo;		/* ival lower bound	*/
    ValNumber up;		/* ival upper bound	*/
    ValNumber step;		/* ival step		*/

    AST_INDEX	loop_ast;	/* AST index of loop header (e.g., DO)	*/
    char	*ivar;		/* str of index variable		*/
				/*	NULL means non-DO loop		*/

    Boolean	fwd;		/* true means (up >= lo) || (step > 0)	*/
				/* false means we just don't know	*/

    Boolean	rev;		/* true means that we flipped up and lo */
				/*	to get a forward loop.		*/
				/* false means we didn't flip -- either */
				/*	because loop was already 	*/
				/* 	forward or we couldn't tell.	*/
} Loop_vals;

#define cfgval_loop_from_ast(cfg, an) \
	(cfgval_loop_from_cfg(cfg, cfg_node_from_ast(cfg, an)))
EXTERN(Loop_vals *, cfgval_loop_from_cfg, (CfgInstance cfg, CfgNodeId cn));
	/*
	 *  Return loop information if this node is actually a loop.
	 */

EXTERN(CoVarPair *, cfgval_dep_parse, (CfgInstance cfg, AST_INDEX sub));


EXTERN(void, cfgval_dep_parse_sub, (CfgInstance cfg, Subs_data *sub, int depth))
;

EXTERN(void, cfgval_dep_parse_loop_bound, (CfgInstance cfg,
					   int         depth,
				           AST_INDEX   bound_ast,
					   Expr       *bound_expr,
					   ValNumber  *bound_val,
				           int       **bound_vec));

#endif /* !cfgval_h */
