/* $Id: ssa_open.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_main.c
 * 
 *        This file contains drivers for SSA construction
 * 
 *        Precondition: CFG construction for the instance must be complete
 *                       before the SSA construction
 */


#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/ssa/ssa_ids.h>
#include <libs/moduleAnalysis/ssa/idfa.h>
#include <libs/support/time/swatch.h>

extern Swatch ssa_OpenOne_sw;
extern Swatch ssa_build_ref_nodes_sw;
extern Swatch ssa_add_ip_refs_sw;
extern Swatch ssa_build_phis_sw;
extern Swatch ssa_convert_phis_sw;

EXTERN(void, ssa_DumpKids, (Generic junk, CfgInstance cfg, CfgNodeId id));

STATIC(void, def_var_cleanup, (Generic fieldVal));

/* 
 * -- ssa_Open
 *
 *	  Add SSA stuff to a CfgInfo.
 */
void ssa_Open(CfgInfo cfgGlobals, Generic ipInfo, Boolean ipSmush, 
              Boolean doArrays, Boolean doDefKill, Boolean doGated)
{
    SsaParms ssaParms;

    ssaParms = (SsaParms) get_mem(sizeof(struct ssa_parms_struct),
				  "Registered parms of SSA analysis");

    ssaParms->ipInfo    = (void *) ipInfo;
    ssaParms->ipSmush   = ipSmush;
    ssaParms->doArrays  = doArrays;
    ssaParms->doDefKill = doDefKill;
    ssaParms->doGated   = doGated;

    cfgGlobals->ssaParms = (Generic) ssaParms;

    /* 
     * Build ssa for each instance separately - skip Global instance 
     */
    cfg_Register(cfgGlobals, (Generic) ssaParms,
		 ssa_CreateOne, ssa_CloseOne, NULL,
		 NULL, ssa_KillKids, ssa_DumpKids,
		 NULL, NULL, NULL);
} /* end of ssa_Open() */



/*
 * -- ssa_Close
 *
 *	Free SSA information only.  Not necessary if also going to do
 *	a cfg_Close, but doesn't hurt then.
 */
void ssa_Close(CfgInfo cfgGlobals )
{
    CfgInstance cfg;

    for (cfg = cfgGlobals->firstInst;
	 cfg;
	 cfg = cfg->next)
    {
	ssa_CloseOne(cfgGlobals->ssaParms, cfg);
    }

    cfg_Deregister(cfgGlobals, cfgGlobals->ssaParms);

    free_mem((void*)cfgGlobals->ssaParms);
    cfgGlobals->ssaParms = (Generic) NULL;

} /* end of ssa_CloseAll() */



/*
 *  ssa_CreateOne
 *
 *  clear fields and call ssa_OpenOne
 */
void ssa_CreateOne(Generic handle, CfgInstance cfg)
{
    CfgNodeId cfgId;

    if ((cfg->type == CFG_GLOBAL) ||
	(cfg->type == CFG_BLOCK_DATA))
	return;

    for (cfgId = cfg_get_first_node(cfg);
	 cfgId != CFG_NIL;
	 cfgId = cfg_get_next_node(cfg, cfgId))
    {
	CFG_node(cfg, cfgId)->ssaKids = SSA_NIL;
    }

    SSA_nodes(cfg)	= (Area) 0;
    SSA_edges(cfg)	= (Area) 0;
    SSA_ipHandle(cfg)	= (Generic) 0;
    SSA_loopPredicates(cfg) = (SsaNodeId *) 0;

    ssa_OpenOne(handle, cfg);
}





/* 
 * -- ssa_OpenOne
 *
 *           Build SSA stuff for an instance 
 */
void ssa_OpenOne(Generic handle, CfgInstance cfg)
{
    if ((cfg->type == CFG_GLOBAL) ||
	(cfg->type == CFG_BLOCK_DATA))
	return;

    swatch_start(ssa_OpenOne_sw);

    /******************
     * initialization *
     ******************/

    if (!cfg->symtab)
	cfg->symtab = (SymDescriptor)
	    ft_SymGetTable((FortTree) cfg->cfgGlobals->ft,
			   cfg_get_inst_name(cfg));

    fst_KillField(cfg->symtab, SSA_VAR_DEFS);

    fst_InitField(cfg->symtab, SSA_VAR_DEFS, (Generic) NULL, def_var_cleanup);

    /*********************
     *    Build SSA      *
     *********************/

    if (SSA_ipInfo(cfg))
	idfaInit(cfg);

    ssa_init_maps(cfg);

    /*
     * First, build the SSA node for real reference nodes.
     * i.e. SSA nodes for real CFG nodes.
     */
    swatch_start(ssa_build_ref_nodes_sw);

    ssa_build_ref_nodes(cfg);

    swatch_lap(ssa_build_ref_nodes_sw);

    /*
     * Second,  add SSA nodes for all common vars to any ENTRY node,
     *          add final RESULT node, etc.
     */
    if (!SSA_ipSmush(cfg))
    {
	swatch_start(ssa_add_ip_refs_sw);

	ssa_add_ip_refs(cfg);

	swatch_lap(ssa_add_ip_refs_sw);
    }

    /*
     * finally, insert all the phi nodes
     */
    swatch_start(ssa_build_phis_sw);

    ssa_build_phis(cfg);

    swatch_lap(ssa_build_phis_sw);

    /*
     *  convert non-loop phis to gammas, if desired
     */
    if (SSA_doGated(cfg)) 
    {
	swatch_start(ssa_convert_phis_sw);

	ssa_convert_phis(cfg);

	swatch_lap(ssa_convert_phis_sw);
    }

    if (SSA_ipInfo(cfg))
	idfaFini(cfg);

    swatch_lap(ssa_OpenOne_sw);

} /* end of ssa_OpenOne() */



/*
 *  Free the SSA information for one CFG instance.
 *  Do not free the CFG instance.
 */
void ssa_CloseOne(Generic handle, CfgInstance cfg)
{
    SsaEdgeId inVarsId;
    int inVarsSize;

    /*
     *  Need to skip this if we never built SSA graph or if it
     *  has already been freed.
     */
    if ((cfg->type == CFG_GLOBAL) ||
	(cfg->type == CFG_BLOCK_DATA) ||
	!(SSA_nodes(cfg)))
	return;

    /*
     *  Not safe to kill field here, symbol table may already be gone.
     *  Symbol table is responsible for calling cleanup function specified
     *  by fst_InitField.
     *
     *  fst_KillField(cfg->symtab, SSA_VAR_DEFS);
     */

    /******************************
     *  Clean up nodes and edges  *
     ******************************/
    ssa_kill_maps(cfg);

    if (SSA_edges(cfg)) area_destroy(&SSA_edges(cfg));
    if (SSA_nodes(cfg)) area_destroy(&SSA_nodes(cfg));

    if (SSA_loopPredicates(cfg)) free_mem((void*) SSA_loopPredicates(cfg));
    SSA_loopPredicates(cfg) = (SsaNodeId *) 0;
}

static void def_var_cleanup(Generic fieldVal)
{
    SsaDefVar *defVar = (SsaDefVar *) fieldVal;
    if (!defVar)
	return;
    
    if (defVar->stack)
	stack_destroy((Stack) defVar->stack);

    free_mem((void*) defVar);
}
