/* $Id: ssa.h,v 3.10 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 *  ssa.h
 *
 *  Public header file for AST/CFG-based Static Single Assignment package.
 *
 */
#ifndef __ssa_h__
#define __ssa_h__

#include <libs/moduleAnalysis/cfg/cfg.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef Generic SsaNodeId;
typedef Generic SsaEdgeId;

#define SSA_NIL (-1)

/*
 *  SSA node types
 *
 *	Special interprocedural nodes
 *
 *	    SSA_IP_MOD
 *		These are treated as "mod" (use and kill) nodes, used
 *		when there may or may not be a definition.
 *
 *	    SSA_IP_KILL
 *		These are treated as kills only, used when must be definition.
 *
 *	    SSA_IP_REF
 *		These are used to represent possible references.
 *
 *	    SSA_ACTUAL
 *		Represent possible uses of call-by-value actual parameters
 *		(expressions).  Should not be simplified to a call-by-ref
 *		actual, if this might result in the ref variable being 
 *		changed.
 *
 *	GIVEN IP INFO (default assume use & mod)
 *		flags:		treatment:
 *
 *		^use, ^mod	no SSA node
 *		use, ^mod	no def node;  SSA_IP_REF
 *		use, mod	SSA_IP_MOD;   SSA_IP_REF
 *		use, kill	SSA_IP_KILL;  SSA_IP_REF
 *		^use, mod	SSA_IP_MOD
 *		^use, kill	SSA_IP_KILL
 *
 *		If there are both use and def (mod/kill), make the use
 *		a child of the def, but occuring earlier in the ref list.
 *		If this is a parameter (there is an ast ref), associate
 *		the astnode with the def, or with the use if no def.
 */
typedef enum {
    /*
     *  Special type for edges...
     */
    SSA_EDGE,

    /*
     *  Use nodes
     */
    SSA_GUARD_LOGICAL,	/* conditions for branches */
    SSA_GUARD_INTEGER,
    SSA_GUARD_INDUCTIVE,
    SSA_GUARD_ALT_RETURN,
    SSA_GUARD_ALT_ENTRY,
    SSA_GUARD_CASE,

    SSA_IP_REF,		/* reference of actual or global at call site */
    SSA_ACTUAL,		/* call-by-value actual; cannot be modified */
    SSA_SUBSCRIPT,	/* array subscript, substring begin or end */
    SSA_ALT_RETURN,	/* Return using alternate return specifier */
    SSA_USE,		/* Other, more vanilla kinds of uses       */

    SSA_LOOP_INIT,      /* \						*/
    SSA_LOOP_BOUND,	/*  > bounds of inductive loop or implied do	*/
    SSA_LOOP_STEP,	/* /						*/

    /*
     *  CALLs are special, use things not modified at call and
     *		have some subordinate definitions.
     */
    SSA_CALL,		/* act as ancestor of references at call site */

    /*
     *  Def nodes
     */
    SSA_IP_IN,		/* formals, COMMON vars, or static vars on entry */
    SSA_INDUCTIVE,	/* loop variable for inductive do or implied do */
    SSA_DEF,		/* assignments or reads (just mod if subscripted def) */

    SSA_IP_KILL,	/* Reference actuals or COMMON vars killed by CALL. */
			/* Not built yet.  Statics also hit if recursion.   */

    /*
     *  Mod nodes (same node is both def and use)
     */
    SSA_PHI,		/* pseudo-def for merge of values */
    SSA_ETA,		/*  " " for index/variant after loop or implied DO */
    SSA_GAMMA,		/* pseudo-def for controlled merge of values */

    SSA_IP_MOD,		/* Ref actuals or COMMON vars modifiable by CALL.   */
			/* Local static vars also hit if recursion allowed. */

    /*
     *  Other mods are SSA_DEF and SSA_IP_KILL, if they kill only part 
     *  of an array.  Such ambiguous defs are treated as "mods" -- 
     *  use and kill.
     */

    SSA_NO_TYPE
} SsaType ;


/*****************************************************************************
 *
 *  SSA (data flow) GRAPH INTERFACE ROUTINES
 *
 *****************************************************************************/



/*****************************************************************************
 *
 *  CREATION and DESTRUCTION
 *
 *  All routines are based on current CFG structures (see cfg.h).
 *
 *  There is a global CFG structure and a CFG instance structure for
 *  each subprogram.  There is an SSA graph for each subprogram, attached
 *  to the CFG instance structure.
 *
 *****************************************************************************/

/*
 *  Routines to create and destroy ssa information.
 *  Should not need to call ssa_*One directly.
 * 
 *  in ssa_main.c
 *
 *  ssa_Open parms:
 *		ipInfo		interprocedural info handle (NULL/0 if none)
 *		ipSmush		true => be dumb about formals and globals
 *		doArrays	true => build SSA nodes and edges for arrays
 *		doDefKill	build edges from defs to killing defs
 *		doGated		build gated single-assignment form: Gammas, etc.
 */
EXTERN(void, ssa_Open,    (CfgInfo globalInfo, Generic ipInfo,
				   Boolean ipSmush, Boolean doArrays,
				   Boolean doDefKill, Boolean doGated));
EXTERN(void, ssa_Close,   (CfgInfo globalInfo));
EXTERN(void, ssa_CreateOne, (Generic junk, CfgInstance cfg));
EXTERN(void, ssa_OpenOne, (Generic junk, CfgInstance cfg));
EXTERN(void, ssa_CloseOne,(Generic junk, CfgInstance cfg));


/*****************************************************************************
 *
 *	Use the cfg*inst routines from cfg.h to get cfgInstance needed for
 *	most SSA routines.
 *
 *****************************************************************************/

/*****************************************************************************
 *
 *  Map an AST node to an SSA node or edge, and vice versa.
 *
 *  Important notes:
 *
 *	-- Mapping is neither one-to-one nor onto.
 *
 *	-- Each AST node maps to an SSA node or edge, or neither,
 *		NEVER BOTH!
 *
 *	-- AST node defining a variable maps to an SSA def node;
 *		this node is unique to this AST.
 *
 *	-- AST node for top-level expression maps to an SSA use node;
 *		this node is unique to this AST.
 *
 *	-- AST node for variable use maps to an SSA edge
 *		(from reaching definition to an SSA use node)
 *		** this edge may be shared by multiple AST nodes
 *		** the SSA use node may be a placeholder for many uses of
 *			different vars
 *****************************************************************************/

/*****************************************************************************
 *
 *  SSA node <-> AST
 *
 */
EXTERN(SsaNodeId, ssa_node_map,       (CfgInstance cfg, AST_INDEX a));
EXTERN(AST_INDEX, ssa_node_to_ast,    (CfgInstance cfg, SsaNodeId sn));

#define ssa_node_exists(cfg, n) (ssa_node_map(cfg, n) != SSA_NIL)

/*****************************************************************************
 *
 *  CFG <-> SSA
 *
 *****************************************************************************/

EXTERN(SsaNodeId,   ssa_get_first_node, (CfgInstance cfg));
	/*
	 *  Returns the id of the first SsaNodeId, such that 
	 *  ssa_get_next_node will cycle through the remaining nodes.
	 */

EXTERN(SsaNodeId,   ssa_get_next_node,
		(CfgInstance cfg, SsaNodeId node));
	/*
	 *  Returns the "next" ssa node after the one passed,
	 *  in Tarjan interval order (inner loops first).
	 *  This will be a topological order (ignoring backedges)
	 *  if tarj_sort(cfg) has been called (see cfg.h).
	 *
	 *  Returns SSA_NIL if there are no more nodes.
	 */

/*
 *  From CFG node (a statement) to its children SSA node. (in ssa_utils.c).
 */
EXTERN(SsaNodeId, ssa_first_cfg_kid,
                (CfgInstance cfg, CfgNodeId cn));

EXTERN(SsaNodeId, ssa_next_cfg_kid,
                (CfgInstance cfg, SsaNodeId sn));

EXTERN(CfgNodeId, ssa_get_cfg_parent,
		(CfgInstance cfg, SsaNodeId sn));


/*****************************************************************************
 *
 *  SSA node hierarchy
 *
 *****************************************************************************/

EXTERN(Boolean, ssa_is_use,  (CfgInstance cfg, SsaNodeId id));
EXTERN(Boolean, ssa_is_kill, (CfgInstance cfg, SsaNodeId id));
EXTERN(Boolean, ssa_is_mod,  (CfgInstance cfg, SsaNodeId id));

#define ssa_is_def(cfg, id)	((id != SSA_NIL) && (!ssa_is_use(cfg, id)))
    /* same as defining to be (ssa_is_kill(cfg, id) || ssa_is_mod(cfg, id)) */

EXTERN(SsaNodeId, ssa_get_ssa_parent,
		(CfgInstance cfg, SsaNodeId sn));
	/*
	 *  From one SSA node to the SSA node for the surrounding expression,
	 *  the containing call, or the LHS (from an RHS).
	 */

EXTERN(SsaNodeId, ssa_first_subuse, (CfgInstance cfg, SsaNodeId sn));
	/*
	 *  The first subordinate SSA use node, if this one is a an expression,
	 *  a call, or an assignment RHS, a subscripted array ref, etc.
	 */
EXTERN(SsaNodeId, ssa_first_subdef, (CfgInstance cfg, SsaNodeId sn));
	/*
	 *  The first subordinate SSA def node, if this one is an SSA_CALL
	 *  or an SSA_GAMMA.
	 */
EXTERN(SsaNodeId, ssa_next_subnode,  (CfgInstance cfg, SsaNodeId sn));
	/*
	 *  The next subordinate def or use node
	 *  (returned node has same def/use categorization as passed node)
	 */

EXTERN(Boolean, ssa_is_guard, (CfgInstance cfg, SsaNodeId ssaId) );
EXTERN(Boolean, ssa_is_loop_type, (CfgInstance cfg, SsaNodeId ssaId) );

/*****************************************************************************
 *  
 *  SSA edges (not necessary to worry about def-kill edges unless asked for)
 *
 *****************************************************************************/

EXTERN(SsaEdgeId,   ssa_first_in,  (CfgInstance cfg, SsaNodeId s));
EXTERN(SsaEdgeId,   ssa_next_in,   (CfgInstance cfg, SsaEdgeId e));

EXTERN(SsaEdgeId,   ssa_first_out, (CfgInstance cfg, SsaNodeId s));
EXTERN(SsaEdgeId,   ssa_next_out,  (CfgInstance cfg, SsaEdgeId e));

/*
 *  These give the outedges to nodes of particular types, respectively, 
 *  kills, uses, and mods (ambiguous defs treated as use/kill combos).
 */
EXTERN(SsaEdgeId,   ssa_first_dk_out, (CfgInstance cfg, SsaNodeId s));
EXTERN(SsaEdgeId,   ssa_first_du_out, (CfgInstance cfg, SsaNodeId s));
EXTERN(SsaEdgeId,   ssa_first_dm_out, (CfgInstance cfg, SsaNodeId s));

EXTERN(SsaEdgeId,   ssa_next_sim_out, (CfgInstance cfg, SsaEdgeId e));

/*
 *  These give the outedges to defs (kills or mods) and to refs (uses or mods)
 */
EXTERN(SsaEdgeId,   ssa_first_def_out, (CfgInstance cfg, SsaNodeId s));
EXTERN(SsaEdgeId,   ssa_first_ref_out, (CfgInstance cfg, SsaNodeId s));

EXTERN(SsaEdgeId,   ssa_next_def_out, (CfgInstance cfg, SsaEdgeId e));
EXTERN(SsaEdgeId,   ssa_next_ref_out, (CfgInstance cfg, SsaEdgeId e));


EXTERN(SsaNodeId,   ssa_edge_src,      (CfgInstance cfg, SsaEdgeId e));
EXTERN(SsaNodeId,   ssa_edge_dest,     (CfgInstance cfg, SsaEdgeId e));

/*
 *  Miscellaneous
 */

EXTERN(SsaNodeId, ssa_first_var_def, (CfgInstance cfg, fst_index_t v));
EXTERN(SsaNodeId, ssa_next_var_def,  (CfgInstance cfg, SsaEdgeId e));
    /*
     *  An arbitrary def to the variable, then other definitions to same
     */

EXTERN(CfgEdgeId,   ssa_edge_pathedge, (CfgInstance cfg, SsaEdgeId e));
    /*
     *  Give the last CFG edge on the control flow path providing this
     *  input to a PHI node
     */

EXTERN(CfgEdgeId,   ssa_edge_label,    (CfgInstance cfg, SsaEdgeId e));
    /*
     *  For an input to a GAMMA node (GSA form), return the corresponding
     *  label of the edge from the controlling predicate.
     */

EXTERN(int,         ssa_node_fanin,    (CfgInstance cfg, SsaNodeId n));
    /*
     *  number of subnodes plus number of reaching defs
     */

EXTERN(int,         ssa_node_fanout,   (CfgInstance cfg, SsaNodeId n));
    /*
     *  only defined for defs
     */

EXTERN(SsaType,     ssa_node_type,     (CfgInstance cfg, SsaNodeId n));
    /*
     *  see SsaType above
     */

EXTERN(fst_index_t, ssa_node_name,     (CfgInstance cfg, SsaNodeId n));
    /*
     *  The symbol table index of the variable referenced (or routine called)
     *  (SYM_INVALID_INDEX if an expression).
     */

EXTERN(void, ssa_stats, (CfgInstance cfg));
	/*
	 * print ssa statistics
	 */

#endif /* ! __ssa_h__ */
