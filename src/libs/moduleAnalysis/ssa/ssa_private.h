/* $Id: ssa_private.h,v 3.10 1997/03/11 14:36:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 
 * -- ssa.h
 * 
 *	    Private include file for SSA constructors
 */



#ifndef __ssa_private_h__
#define __ssa_private_h__

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/ssa/areas.h>
#include <libs/support/stacks/xstack.h>
#include <libs/support/sets/set.h>
#include <assert.h>

#define SSA_NIL_NAME	(SYM_INVALID_INDEX)

/*
 *  Used in Gamma dags for loop continuation predicates
 */
#define SSA_LOOP_TRUE	(-3)
#define SSA_LOOP_FALSE	(-2)

/*
 *  Least-sleazy way of making sure that two structures start out with
 *  the same fields in same order.
 *
 *  SsaUse's and SsaDef's are stored in the same data structure and have
 *  overlapping index spaces (distinguished by type field only).
 *
 *  Note that fanIn is from #subUses + #defsIn, even though this doesn't
 *  make much sense for kills, especially when doDefKill is on.
 *
 *  Don't put the type first, so that we can tell when it's been clobbered.
 */
typedef struct ssa_shared_struct {
    AST_INDEX refAst;		/* ASTnode # of reference */
    fst_index_t name;		/* var referenced or procedure called */
    SsaType type;		/* Type (of def or use) */
    CfgNodeId cfgParent;	/* CFG node expr is assoc'd with */
    SsaNodeId nextCfgSib;	/* Next SSA child of cfgParent */
    SsaNodeId ssaParent;	/* SSA node this node is subordinate to */
    SsaNodeId nextSsaSib;	/* Next SSA child of ssaParent */
    SsaNodeId subUses;		/* 1st ssa use node subordinate to this node */
    Generic   value;		/* value number for the expression */
    SsaEdgeId defsIn;		/* first edge for reaching definition */
    SmallInt unuseful;		/* flag whether expr has useful out edges */
    SmallInt fanIn;		/* # of inputs (#subUses + #defsIn) */
} SsaShared;


/*
 *  SSA definition node
 *
 *  Definitions include both kills and mods.  Mods are ambiguous defs
 *  or other defs which must also be treated as uses.
 *
 *  There are separate lists at the source definition for def-kill,
 *  def-mod, and def-use edges, because def-use and def-kill edges need
 *  to be handled differently, and def-mod edges need to be a bit of both.
 *  There's no need to distinguish at the sink, because all are sourced
 *  at definitions.
 *
 *  Types of "mods":
 *
 *	* phi-nodes, IND_KILLERs
 *	* subscripted array definitions
 *	* assumed mods at call sites SSA_IP_MOD (vs. definite mods SSA_IP_KILL)
 *	* aliases and pointers, when they are handled
 */
typedef struct ssa_def_struct {
    SsaShared stuff;		/* fields shared with SsaUse */
    SsaEdgeId defKillOuts;	/* first edge to following certain def */
    SsaEdgeId defModOuts;	/* first edge to following ambiguous def */
    SsaEdgeId defUseOuts;	/* first edge to reachable use */
    SsaNodeId nextDef;		/* another definition to same variable */
} SsaDef;

/*
 *  If "valueOut" is SSA_NIL, then the use is hung directly off "cfgNode"
 *  and "next" is next use so directly hung.
 *
 *  If "valueOut" is not SSA_NIL, then use is hung off the indexed
 *  SsaUse or SsaDef and "next" is next use hung off same SsaUse or SsaDef.
 */
typedef struct ssa_use_struct {
    SsaShared stuff;		/* fields shared with SsaDef */
    SsaNodeId subDefs;		/* first subordinate def (of an SSA_CALL) */
} SsaUse;

/*
 *  SsaNode structure has size max(sizeof(SsaUse), sizeof(SsaDef));
 *  is used for the element size of the SsaNode table.
 */
typedef struct ssa_node_struct {
    union {
	struct ssa_def_struct Ssa_Def;
	struct ssa_use_struct Ssa_Use;
    } which;
} SsaNode;

/*
 *  Edge from definition (def or mod) to use, def, or mod.
 */
typedef struct ssa_edge_struct {
    SsaNodeId source;	/* ssa node defining var */
    SsaNodeId sink;	/* ssa node using and/or redefining var */
    CfgEdgeId pathEdge;	/* last edge on cfg path to merge */
    SsaEdgeId inNext;	/* next edge in "in" list of sink node */
    SsaEdgeId outNext;	/* next edge in "out" list of source node */
} SsaEdge ;


/*
 *  information for a variable defined in a CfgInstance.
 */
typedef struct ssa_ref_var {
    fst_index_t var;	/* symtab index of variable */
    Set work;		/* CfgNodes added to def_set*/
    Set DFP;		/* CfgNodes in iterated dominance frontier */
    SsaNodeId defList;	/* first in list of defining expressions for var */
    Stack stack;	/* xstack; top is current def (in building ssa) */
} SsaDefVar;

#define SSA_VAR_DEFS	"SSA variable defs: (SsaDefVar *)"

typedef struct ssa_parms_struct {
    void *ipInfo;	/* handle for interprocedural info */
    Boolean ipSmush;	/* treat IP vars (formals/globals) as DUMMY_GLOBAL? */
    Boolean doArrays;	/* build edges for arrays? */
    Boolean doDefKill;	/* build edges from defs to killing defs? */
    Boolean doGated;	/* build Gated Single-Assignment form? */
} * SsaParms;

/* 
 *  Space for a few pointers to SSA stuff is reserved in each
 *  CfgInstance object.  The size of this structure must be kept within
 *  the amount of space reserved.
 */
typedef struct ssa_stuff_struct {
    Area	ssaNodes;
    Area	ssaEdges;
    SsaNodeId * loopPredicates;
    Generic	ipHandle;
    void *	ssaNodeMap;	/* map AST_INDEX to SsaNodeId */
} * SsaStuff;

/*
 *  Induce a compilation error if ssa_stuff_struct is too big
 *  -- array dimension cannot be zero or negative.
 */
char ssa_foo_junk[(SSA_WORK_SLOTS * sizeof(Generic)) -
		  sizeof(struct ssa_stuff_struct) +1];

#define SSA_nodes(cfg)		(((SsaStuff)cfg->ssaStuff)->ssaNodes)
#define SSA_edges(cfg)		(((SsaStuff)cfg->ssaStuff)->ssaEdges)
#define SSA_node(cfg, id)	((SsaShared *)area_addr(SSA_nodes(cfg), id))
#define SSA_def(cfg, id)	((SsaDef *)SSA_node(cfg, id))
#define SSA_use(cfg, id)	((SsaUse *)SSA_node(cfg, id))
#define SSA_edge(cfg, id)	((SsaEdge *)area_addr(SSA_edges(cfg), id))
#define SSA_ipHandle(cfg)	(((SsaStuff)cfg->ssaStuff)->ipHandle)

#define SSA_loopPredicates(cfg)	(((SsaStuff)cfg->ssaStuff)->loopPredicates)

#define SSA_parms(cfg)		((SsaParms)(cfg->cfgGlobals->ssaParms))
#define SSA_ipInfo(cfg)		((SSA_parms(cfg))->ipInfo)
#define SSA_ipSmush(cfg)	((SSA_parms(cfg))->ipSmush)
#define SSA_doArrays(cfg)	((SSA_parms(cfg))->doArrays)
#define SSA_doDefKill(cfg)	((SSA_parms(cfg))->doDefKill)
#define SSA_doGated(cfg)	((SSA_parms(cfg))->doGated)

/*
 *  New macro for variable id now that string table doesn't give
 *  same id for repeated occurrences.
 */
#define SsaGetSym(cfg, astnode) \
    (fst_QueryIndex((cfg)->symtab, gen_get_text(astnode)))

#define DUMMY_GLOBAL(cfg) (fst_Index((cfg)->symtab, "DUMMY_GLOBAL"))

#define SsaSymText(cfg, symId) \
    ((char *) fst_GetFieldByIndex((cfg)->symtab, symId, SYMTAB_NAME))
/*
 *  ast_recognizers for special expression types -
 */
/* constant true and false values */
#define CONST_TRUE ".true."
#define CONST_FALSE ".false."

/*
 *  Main driver routines...
 */
EXTERN(void, ssa_build_ref_nodes, (CfgInstance cfg));

EXTERN(void, ssa_add_ip_refs, (CfgInstance cfg) );

EXTERN(void, ssa_build_phis, (CfgInstance cfg) );

EXTERN(void, ssa_convert_phis, (CfgInstance cfg) );
    
/*
 *  Utility routines...
 */

#define ssa_is_special_def(cfg, id)    \
	((SSA_node(cfg, id)->type == SSA_ETA) || \
	 (SSA_node(cfg, id)->type == SSA_PHI))

EXTERN(Boolean, ssa_ignored,
		(CfgInstance cfg, SsaNodeId ssaId));

EXTERN(Boolean, ssa_is_subarray_type,
		(CfgInstance cfg, SsaNodeId ssaId));

EXTERN(SsaDefVar *, ssa_find_ref_slot,
		(CfgInstance cfg, fst_index_t var));

EXTERN(Boolean, ssa_has_defs,
		(CfgInstance cfg, fst_index_t var));

EXTERN(SsaNodeId, ssa_init_node,
		(CfgInstance	cfg,
		 CfgNodeId	cfgId,
		 SsaNodeId	ssaParent,
		 AST_INDEX	astId,
		 SsaType	type,
		 fst_index_t	var));

EXTERN(fst_index_t, ssa_get_ind_var,
		(CfgInstance cfg, CfgNodeId cfgId));

EXTERN(fst_index_t, ssa_var_rep, (CfgInstance cfg, fst_index_t var));

EXTERN(fst_index_t, ssa_var_rep_extent, (CfgInstance cfg, 
					 fst_index_t var,
					 int *extent));

EXTERN(fst_index_t, ssa_node_rep, (CfgInstance cfg, SsaNodeId sn));

EXTERN(Boolean, ssa_var_covers_eq, (CfgInstance cfg, fst_index_t var));

EXTERN(void, ssa_DumpKids, (Generic junk, CfgInstance cfg, CfgNodeId cfgNode));

#endif /* !__ssa_private_h__ */
