/* $Id: cfg_changes.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_changes.c
 *
 */

#include <libs/moduleAnalysis/cfg/cfg_private.h>
#include <string.h>

void cfg_TreeWillChange(CfgInfo cfgGlobals, AST_INDEX node)
{
    CfgInstance cfg = cfgGlobals->firstInst;

    if (cfgGlobals->sideArray && cfg)
    {
	if (cfg_node_map(cfg, node) != CFG_NIL)
	{
	    /*
	     *  Get the containing subprogram
	     */
	    for (;
		 !(is_null_node(node) || new_instance(node));
		 node = out(node))
		;

	    /*
	     *  Get the corresponding CfgInstance
	     */
	    for (;
		 cfg->next && (cfg->astnode != node);
		 cfg = cfg->next)
		;

	    /*
	     *  If we have the appropriate CfgInstance, destroy it
	     */
	    if (cfg->astnode == node)
	    {
		cfg_destroy_inst(cfg);
	    }
	}
    }
}

void cfg_TreeChanged(CfgInfo cfgGlobals, AST_INDEX node)
{
    return;
}
