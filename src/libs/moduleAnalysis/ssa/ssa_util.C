/* $Id: ssa_util.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>
#include <libs/moduleAnalysis/ssa/idfa.h>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>

STATIC(void, insert_def, (CfgInstance cfg, SsaNodeId ssaId));
STATIC(SsaDefVar*, get_ref_slot, (CfgInstance cfg, fst_index_t var));
STATIC(CfgNodeId, next_tarj_cn, (CfgInstance cfg, CfgNodeId cn));

/*
 *  True is node should be treated as a use.
 */
Boolean ssa_is_use(CfgInstance cfg, SsaNodeId n)
{
    if (n == SSA_NIL) return false;

    return BOOL(!ssa_is_mod(cfg, n) && !ssa_is_kill(cfg, n));
}

/*
 *  True if node should be treated as a killing (unambiguous) definition.
 */
Boolean ssa_is_kill(CfgInstance cfg, SsaNodeId n)
{
    fst_index_t var, rep;
    int extent;

    if (n == SSA_NIL) return false;

    switch (SSA_node(cfg, n)->type)
    {
	/*  
	 *  First three cases depends on whether variable covers all 
	 *  overlapping equivalenced variables.
	 */
      case SSA_DEF:
      case SSA_IP_KILL:
      case SSA_INDUCTIVE:
	return NOT(ssa_is_mod(cfg, n));
	break;

      case SSA_IP_IN:
	return true;

      default:
	return false;
    }
}


/*
 *  True if node should be treated as a use and def both
 *  (ambiguous def or pseudo-assignment)
 */
Boolean ssa_is_mod(CfgInstance cfg, SsaNodeId n)
{
    if (n == SSA_NIL) return false;

    switch (SSA_node(cfg, n)->type)
    {
      case SSA_DEF:
	if (ssa_is_subarray_type(cfg, n)) return true;
	/*
	 *  deliberate fallthrough
	 */
      case SSA_IP_KILL:
      case SSA_INDUCTIVE:
	return BOOL(!ssa_var_covers_eq(cfg, SSA_node(cfg, n)->name));

      case SSA_PHI:
      case SSA_ETA:
      case SSA_GAMMA:
      case SSA_IP_MOD:
	return true;

      default:
	return false;
    }
}



/* 
 * -- insert_def
 *
 *	 insert defining expr to var in SsaDefVar list for inst and 
 *	 any surrounding instances 
 */
static void insert_def(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaDefVar *defVar = NULL;

    if (SSA_node(cfg, ssaId)->name == SSA_NIL_NAME) return;

    if (!ssa_ignored(cfg, ssaId))
    {
	/*
	 *  Find representative of potentially equivalenced var
	 */
	fst_index_t var = ssa_node_rep(cfg, ssaId);

	defVar = get_ref_slot(cfg, var);
    }

    if (!defVar) return;

    SSA_def(cfg, ssaId)->nextDef = defVar->defList;
    defVar->defList = ssaId;
    
} /* end of insert_def() */





/* 
 * -- get_ref_slot
 *
 *	initialize and return SsaDefVar structure for var 
 *	-- also used to mark variable as used
 */
static SsaDefVar *get_ref_slot(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *slot;

    if (!(var == SSA_NIL_NAME) &&
	SSA_ipSmush(cfg) &&
	FS_IS_IP_VAR(cfg->symtab, var))
    {
	var = DUMMY_GLOBAL(cfg);
    }
    slot = ssa_find_ref_slot(cfg, var);
    
    if (slot) /* already exists */
	return(slot);

    /* else insert */
    slot = (SsaDefVar *) get_mem(sizeof(SsaDefVar), "get_ref_slot");
    slot->var  = var;
    slot->work = NULL;
    slot->DFP  = NULL;
    slot->defList = SSA_NIL;
    slot->stack   = stack_create(sizeof(SsaNodeId));
    
    fst_PutFieldByIndex(cfg->symtab, var, SSA_VAR_DEFS, (Generic) slot);

    return (slot);

} /* end of get_ref_slot() */




/* 
 * -- ssa_find_ref_slot
 *
 *	lookup SsaDefVar structure for var 
 */
SsaDefVar *ssa_find_ref_slot(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *slot;
    fst_index_t rep;

    if (var == SSA_NIL_NAME) return NULL;

    rep = ssa_var_rep(cfg, var);

    slot = (SsaDefVar *)fst_GetFieldByIndex(cfg->symtab, rep, SSA_VAR_DEFS);

    return(slot);

} /* end of ssa_find_ref_slot() */



Boolean ssa_has_defs(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *slot;
    Generic sc;

    slot = ssa_find_ref_slot(cfg, var);

    if (slot && (slot->defList != SSA_NIL))
	return true;

    sc = fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_STORAGE_CLASS);

    if (SSA_ipInfo(cfg) && (sc & SC_GLOBAL))
    {
	/*
	 *  Returns true if modified via call to subprogram
	 *  regardless of entry point or whether mod is direct or indirect.
	 */
	return idfaNameIsMod(cfg, AST_NIL, var);
    }
    else
	return false;
}




/****************************************************************************/



/*
 * -- ssa_is_guard
 *
 *       Return true if the ssa type is a guard type
 *       otherwise return false
 */
Boolean ssa_is_guard(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaType type;

    type = SSA_node(cfg, ssaId)->type;

    switch(type)
    {
      case SSA_GUARD_LOGICAL:
      case SSA_GUARD_INTEGER:
      case SSA_GUARD_INDUCTIVE:
      case SSA_GUARD_ALT_RETURN:
      case SSA_GUARD_ALT_ENTRY:
      case SSA_GUARD_CASE:
	return(true);

      default:
	return(false);
    }
} /* end of ssa_is_guard() */




/*
 *  ssa_ignored()
 *
 *  returns true if the node should not be treated as referencing its
 *  variable -- because that variable is an array etc.
 *
 *  Currently true only if doArrays is false, and node
 *  is an array reference or a ref to an equiv'd var not covered.
 */
Boolean ssa_ignored(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaShared *ptr = SSA_node(cfg, ssaId);

    if (SSA_doArrays(cfg))
	return false;

    if (is_identifier(ptr->refAst))
    {
	fst_index_t rep;
	int extent;

	if (FS_IS_ARRAY(cfg->symtab, SsaGetSym(cfg, ptr->refAst)))
	{
	    return true;
	}
	/*
	 *  Return true if the representative of this variable
	 *  doesn't cover the extent of the equivalenced set
	 */
	return BOOL(!ssa_var_covers_eq(cfg, ssa_node_rep(cfg, ssaId)));
    }
    if (is_subscript(ptr->refAst) || is_substring(ptr->refAst))
    {
	return(true);
    }
    return(false);

} /* end of ssa_ignored() */



/*
 *  Boolean ssa_is_subarray_type()
 *
 *  returns true if the node is a subarray reference
 */
Boolean ssa_is_subarray_type(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaShared *ptr = SSA_node(cfg, ssaId);

    if (is_subscript(ptr->refAst) || is_substring(ptr->refAst))
    {
	return(true);
    }
    return(false);

} /* end of ssa_is_subarray_type() */



/*
 * -- ssa_is_loop_type
 *
 *       Return true if the ssa type is a loop type
 *       otherwise return false
 */
Boolean ssa_is_loop_type(CfgInstance cfg, SsaNodeId ssaId)
{
    SsaType type;

    type = SSA_node(cfg, ssaId)->type;

    switch(type)
    {
      case SSA_LOOP_STEP:
      case SSA_LOOP_BOUND:
      case SSA_LOOP_INIT:
	return(true);

      default:
	return(false);
    }
} /* end of ssa_is_loop_type() */




/*
 * -- ssa_init_node
 *
 *	  Allocate a new SSA node and initialize it
 */
SsaNodeId ssa_init_node(CfgInstance cfg, CfgNodeId cfgId, SsaNodeId ssaParent, 
                        AST_INDEX astId, SsaType type, fst_index_t var)
{
    SsaNodeId New;
    SsaShared *newPtr;

    if (ssaParent != SSA_NIL) cfgId = SSA_node(cfg, ssaParent)->cfgParent;

    /*
     *  Make sure ip vars are replaced with DUMMY_GLOBAL
     */
    if (!(var == SSA_NIL_NAME) &&
	SSA_ipSmush(cfg) &&
	FS_IS_IP_VAR(cfg->symtab, var))
    {
	var = DUMMY_GLOBAL(cfg);
    }
    New = ssa_node_new_id(cfg, type);
    newPtr = SSA_node(cfg, New);
    newPtr->refAst     = astId;
    newPtr->cfgParent  = cfgId;
    newPtr->ssaParent  = ssaParent;
    newPtr->name       = var;

    /* *-> HACK ALERT <-*
     * 
     * Strictly speaking, the ssa stuff shouldn't have to know anything about
     * value numbering, but unless we initialize this field to -1, it will
     * be misinterpreted by the value numbering stuff. This is a huge hack
     * (relies on the fact that SSA_NIL is currently = VAL_NIL_INDEX).
     *
     * -Nat McIntosh
     * 
     * Value numbering initializes this field, but I'll leave this here
     * to comfort those of little faith. :-)
     * --paco
     */
    newPtr->value      = SSA_NIL;
    
    if ((type != SSA_PHI) ||
	(ssa_get_ind_var(cfg, cfgId) == SSA_NIL_NAME))
    {
	newPtr->nextCfgSib = CFG_node(cfg, cfgId)->ssaKids;
	CFG_node(cfg, cfgId)->ssaKids = New;
    }
    else
    {
	SsaNodeId tmp;

	/*
	 *  Need to put SSA_PHI after loop ssa nodes
	 *
	 *  there must be an SSA_INDUCTIVE node after all the other loop nodes
	 */
	for (tmp = CFG_node(cfg, cfgId)->ssaKids;
	     SSA_node(cfg, tmp)->type != SSA_INDUCTIVE;
	     tmp = SSA_node(cfg, tmp)->nextCfgSib)
	    ;
	newPtr->nextCfgSib = SSA_node(cfg, tmp)->nextCfgSib;
	SSA_node(cfg, tmp)->nextCfgSib = New;
    }


    if (ssa_is_use(cfg, New))
    {
	if (ssaParent != SSA_NIL)
	{
	    newPtr->nextSsaSib = SSA_node(cfg, ssaParent)->subUses;
	    SSA_node(cfg, ssaParent)->subUses = New;

	    (SSA_node(cfg, ssaParent)->fanIn)++;
	}
    }
    else /* is_def == is_kill || is_mod */
    {
	if (ssaParent != SSA_NIL)
	{
	    SsaType pType = SSA_node(cfg, ssaParent)->type;

	    if ((pType != SSA_CALL) &&
		(pType != SSA_GUARD_INDUCTIVE))
	    {
		fprintf(stderr, "Bogus ssaParent for def in ssa_init_node\n");
		ssaParent = SSA_NIL;
	    }
	    else
	    {
		newPtr->nextSsaSib = SSA_use(cfg, ssaParent)->subDefs;
		SSA_use(cfg, ssaParent)->subDefs = New;
	    }
	}
	if (!ssa_ignored(cfg, New))
	{
	    insert_def(cfg, New);
	}
    }

    /*
     *  Log map from astnode to ssa node (other way is by refAst field)
     *
     *  Need to maintain invariant that an ast node can appear in the refAst
     *  field of only one ssa node (other ssa nodes are handles with AST_NIL).
     */
    if (!is_null_node(astId)) ssa_node_put_map(cfg, astId, New);

    return New;
} /* end of ssa_init_node() */




/*
 * -- ssa_get_ind_var
 *
 *        Given a inductive loop statement CFG node index,
 *        returns the symbol table index of the inductive variable
 *        SSA_NIL_NAME is returned otherwise
 */
fst_index_t ssa_get_ind_var(CfgInstance cfg, CfgNodeId cfgId)
{
    AST_INDEX astId, astControl;
    NODE_TYPE astType;
    fst_index_t indVar;

    astId = CFG_node(cfg, cfgId)->astnode;
    astType = ast_get_node_type(astId);
    switch (astType)
    {
      case GEN_DO:
        astControl = gen_DO_get_control(astId);
        break;

      case GEN_DO_ALL:
        astControl = gen_DO_ALL_get_control(astId);
        break;

      case GEN_PARALLELLOOP:
        astControl = gen_PARALLELLOOP_get_control(astId);
        break;

      default: /* not a is_loop */
        return( SSA_NIL_NAME );
    }

    if ( is_inductive(astControl) )
        indVar = SsaGetSym(cfg, gen_INDUCTIVE_get_name(astControl));
    else
        indVar = SSA_NIL_NAME;

    return(indVar);

} /* end of ssa_get_ind_var() */


/*
 *  Get next ssa node in tarjan-interval order on CFG
 */
SsaNodeId ssa_get_next_node(CfgInstance cfg, SsaNodeId sn)
{
    CfgNodeId cn;

    if (sn != SSA_NIL)
    {
	SsaNodeId follow;

	follow = SSA_node(cfg, sn)->nextCfgSib;
	if (follow != SSA_NIL)
	    return follow;

	cn = next_tarj_cn(cfg, SSA_node(cfg, sn)->cfgParent);
    }
    else
	cn = cfg->start;

    while ((cn != CFG_NIL) &&
	   (CFG_node(cfg, cn)->ssaKids == SSA_NIL))
    {
	cn = next_tarj_cn(cfg, cn);
    }
    if (cn == CFG_NIL)
	return SSA_NIL;
    else
	return CFG_node(cfg, cn)->ssaKids;
}

SsaNodeId ssa_get_first_node(CfgInstance cfg)
{
    if (!(cfg->topMap)) tarj_sort(cfg);

    return ssa_get_next_node(cfg, SSA_NIL);
}

/*
 *  Returns next cfg node in treewalk order on the tarjan interval tree.
 */
static CfgNodeId next_tarj_cn(CfgInstance cfg, CfgNodeId cn)
{
    CfgNodeId loop;

    if (TARJ_inners(cfg->intervals, cn) != CFG_NIL)
	return TARJ_inners(cfg->intervals, cn);

    loop = TARJ_outer(cfg->intervals, cn);

    while ((TARJ_next(cfg->intervals, cn) == CFG_NIL) &&
	   (loop != CFG_NIL))
    {
	cn = loop;
	loop = TARJ_outer(cfg->intervals, cn);
    }
    return TARJ_next(cfg->intervals, cn);
}


/*
 *  Functions invented for external use, see ssa.h
 */

AST_INDEX ssa_node_to_ast(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->refAst;
}

int ssa_edge_ast_uses(CfgInstance cfg, SsaEdgeId se)
{
    SsaNodeId sink = SSA_edge(cfg, se)->sink;
    SsaType type = SSA_node(cfg, sink)->type;
    int occurs;

    if ((type == SSA_GAMMA) || (type == SSA_PHI))
	return 0;

    occurs = SSA_edge(cfg, se)->pathEdge;

    if (occurs == CFG_NIL)
	return 0;
    else
	return occurs;
}

SsaNodeId ssa_first_cfg_kid(CfgInstance cfg, CfgNodeId cn)
{
    return CFG_node(cfg, cn)->ssaKids;
}

SsaNodeId ssa_next_cfg_kid(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->nextCfgSib;
}

CfgNodeId ssa_get_cfg_parent(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->cfgParent;
}
    
SsaNodeId ssa_get_ssa_parent(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->ssaParent;
}

SsaNodeId ssa_first_subuse(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->subUses;
}

SsaNodeId ssa_first_subdef(CfgInstance cfg, SsaNodeId sn)
{
    if (ssa_is_def(cfg, sn))
	return SSA_NIL;

    return SSA_use(cfg, sn)->subDefs;
}

SsaNodeId ssa_next_subnode(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->nextSsaSib;
}

SsaNodeId ssa_first_var_def(CfgInstance cfg, fst_index_t var)
{
    SsaDefVar *slot = ssa_find_ref_slot(cfg, var);
    if (!slot)
	return SSA_NIL;
    else    
	return slot->defList;
}

SsaNodeId ssa_next_var_def(CfgInstance cfg, SsaNodeId sn)
{
    if (ssa_is_use(cfg, sn))
	return SSA_NIL;
    else
	return SSA_def(cfg, sn)->nextDef;
}

SsaEdgeId ssa_first_in(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->defsIn;
}

SsaEdgeId ssa_next_in(CfgInstance cfg, SsaEdgeId se)
{
    return SSA_edge(cfg, se)->inNext;
}

SsaEdgeId ssa_first_out(CfgInstance cfg, SsaNodeId sn)
{
    SsaEdgeId retVal;

    if (ssa_is_use(cfg, sn))
	return SSA_NIL;

    retVal = SSA_def(cfg, sn)->defUseOuts;
    if (retVal != SSA_NIL)
	return retVal;
    else
	return ssa_first_def_out(cfg, sn);
}

SsaEdgeId ssa_next_out(CfgInstance cfg, SsaEdgeId se)
{
    SsaEdgeId retVal = SSA_edge(cfg, se)->outNext;
    SsaNodeId src, sink;

    if (retVal != SSA_NIL)
	return retVal;

    src  = SSA_edge(cfg, se)->source;
    sink = SSA_edge(cfg, se)->sink;

    if (ssa_is_use(cfg, sink))
	return ssa_first_def_out(cfg, src);

    if (!ssa_is_kill(cfg, sink))
	return SSA_def(cfg, src)->defKillOuts;

    return SSA_NIL;
}

SsaEdgeId ssa_first_dk_out(CfgInstance cfg, SsaNodeId sn)
{
    if (ssa_is_use(cfg, sn)) return SSA_NIL;

    return SSA_def(cfg, sn)->defKillOuts;
}

SsaEdgeId ssa_first_du_out(CfgInstance cfg, SsaNodeId sn)
{
    if (ssa_is_use(cfg, sn)) return SSA_NIL;

    return SSA_def(cfg, sn)->defUseOuts;
}

SsaEdgeId ssa_first_dm_out(CfgInstance cfg, SsaNodeId sn)
{
    if (ssa_is_use(cfg, sn)) return SSA_NIL;

    return SSA_def(cfg, sn)->defModOuts;
}

SsaEdgeId ssa_next_sim_out(CfgInstance cfg, SsaEdgeId se)
{
    return SSA_edge(cfg, se)->outNext;
}

SsaEdgeId ssa_first_def_out(CfgInstance cfg, SsaNodeId sn)
{
    SsaEdgeId retVal;

    if (ssa_is_use(cfg, sn))
	return SSA_NIL;

    retVal = SSA_def(cfg, sn)->defModOuts;
    if (retVal != SSA_NIL)
	return retVal;

    return SSA_def(cfg, sn)->defKillOuts;
}
SsaEdgeId ssa_next_def_out(CfgInstance cfg, SsaEdgeId se)
{
    SsaEdgeId retVal = SSA_edge(cfg, se)->outNext;
    SsaNodeId src, sink;

    if (retVal != SSA_NIL)
	return retVal;

    src  = SSA_edge(cfg, se)->source;
    sink = SSA_edge(cfg, se)->sink;

    if (!ssa_is_kill(cfg, sink))
	return SSA_def(cfg, src)->defKillOuts;

    return SSA_NIL;
}

SsaEdgeId ssa_first_ref_out(CfgInstance cfg, SsaNodeId sn)
{
    SsaEdgeId retVal;

    if (ssa_is_use(cfg, sn))
	return SSA_NIL;

    retVal = SSA_def(cfg, sn)->defUseOuts;
    if (retVal != SSA_NIL)
	return retVal;

    return SSA_def(cfg, sn)->defModOuts;
}
SsaEdgeId ssa_next_ref_out(CfgInstance cfg, SsaEdgeId se)
{
    SsaEdgeId retVal = SSA_edge(cfg, se)->outNext;
    SsaNodeId src, sink;

    if (retVal != SSA_NIL)
	return retVal;

    src  = SSA_edge(cfg, se)->source;
    sink = SSA_edge(cfg, se)->sink;

    if (ssa_is_use(cfg, sink))
	return SSA_def(cfg, src)->defModOuts;

    return SSA_NIL;
}


SsaNodeId ssa_edge_src(CfgInstance cfg, SsaEdgeId se)
{
    return SSA_edge(cfg, se)->source;
}

SsaNodeId ssa_edge_dest(CfgInstance cfg, SsaEdgeId se)
{
    return SSA_edge(cfg, se)->sink;
}

CfgEdgeId ssa_edge_pathedge(CfgInstance cfg, SsaEdgeId se)
{
    SsaNodeId sink = SSA_edge(cfg, se)->sink;
    SsaType   type = SSA_node(cfg, sink)->type;

    if (type != SSA_PHI)
	return CFG_NIL;
    else
	return SSA_edge(cfg, se)->pathEdge;
}

CfgEdgeId ssa_edge_label(CfgInstance cfg, SsaEdgeId se)
{
    SsaNodeId sink = SSA_edge(cfg, se)->sink;
    SsaType   type = SSA_node(cfg, sink)->type;

    if (type != SSA_GAMMA)
	return CFG_NIL;
    else
	return SSA_edge(cfg, se)->pathEdge;
}

int ssa_node_fanin(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->fanIn;
}

int ssa_node_fanout(CfgInstance cfg, SsaNodeId sn)
{
    int count;
    SsaEdgeId se;

    if (SSA_node(cfg, sn)->ssaParent != SSA_NIL)
	count = 1;
    else
	count = 0;

    if (ssa_is_use(cfg, sn))
	return count;

    for (se = SSA_def(cfg, sn)->defKillOuts;
	 se != SSA_NIL;
	 se = SSA_edge(cfg, se)->outNext) count++;

    for (se = SSA_def(cfg, sn)->defModOuts;
	 se != SSA_NIL;
	 se = SSA_edge(cfg, se)->outNext) count++;

    for (se = SSA_def(cfg, sn)->defUseOuts;
	 se != SSA_NIL;
	 se = SSA_edge(cfg, se)->outNext) count++;

    return count;
}

SsaType ssa_node_type(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->type;
}

fst_index_t ssa_node_name(CfgInstance cfg, SsaNodeId sn)
{
    return SSA_node(cfg, sn)->name;
}

fst_index_t ssa_var_rep_extent(CfgInstance cfg, fst_index_t var, int* extent)
{
    fst_index_t rep;

    if ((var == SSA_NIL_NAME) || 
	(var == DUMMY_GLOBAL(cfg)) ||
	!fst_EquivSuperClassByIndex(cfg->symtab, var, &rep, extent))
    {
	*extent = INFINITE_INTERVAL_LENGTH;
	rep = var;
	/*
	 *  Otherwise, these are set by the function call...
	 */
    }
    return rep;
}

Boolean ssa_var_covers_eq(CfgInstance cfg, fst_index_t var)
{
    fst_index_t rep;
    int extent;

    if (var == DUMMY_GLOBAL(cfg)) return false;

    rep = ssa_var_rep_extent(cfg, var, &extent);

    if (fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_SIZE) == extent)
	return true;
    else
	return false;
}


fst_index_t ssa_var_rep(CfgInstance cfg, fst_index_t var)
{
    int extent;
    return ssa_var_rep_extent(cfg, var, &extent);
}


fst_index_t ssa_node_rep(CfgInstance cfg, SsaNodeId sn)
{
    return ssa_var_rep(cfg, SSA_node(cfg, sn)->name);
}
