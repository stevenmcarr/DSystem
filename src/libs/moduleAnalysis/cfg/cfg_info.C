/* $Id: cfg_info.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_info.c
 *
 */

#include <assert.h>
#include <libs/moduleAnalysis/cfg/cfg_private.h>

#define ast_2_info(cfg, n) \
    (ft_GetFromSideArray((Generic) cfg->cfgGlobals->sideArray, n, 0))

#define put_ast_info(cfg,n,v) \
    (ft_PutToSideArray((Generic) cfg->cfgGlobals->sideArray, n, 0, v))

STATIC(Boolean, check_cfg_node,(CfgInstance cfg, AST_INDEX node,
                                CfgNodeId id));

void cfg_put_map(CfgInstance cfg, AST_INDEX n, Generic v, 
                 CfgInfoType type)
{
    assert ((v == CFG_NIL) || (ast_2_info(cfg, n) == CFG_NIL));
    
    put_ast_info(cfg, n, v);
}

Generic cfg_map(CfgInstance cfg, AST_INDEX n, CfgInfoType type)
{
    int i;

    if (type == CFG_INFO_EDGE)
	return CFG_NIL;

    if (type != CFG_INFO_NODE)
    {
	return ast_2_info(cfg, n);
    }

    if (is_loop(n))
    {
	i = (int) cfg_preheader_map(cfg, n);
	if (i != CFG_NIL)
	    return i;
    }

    i = ast_2_info(cfg, n);

    if (check_cfg_node(cfg, n, i))
	return i;
    else
	return CFG_NIL;
}

/*
 *  For this subroutine, the AST node must be a DO loop statement
 *  or the label on a DO loop statement (not the end label).
 *
 *  The preheader node is for the label def, the header for the loop itself.
 */
CfgNodeId cfg_header_map(CfgInstance cfg, AST_INDEX n)
{
    int i;

    if (!is_loop(n))
    {
	n = out(n);
	if (!is_loop(n)) return CFG_NIL;
    }

    i = ast_2_info(cfg, n);

    if (check_cfg_node(cfg, n, i))
	return i;
    else
	return CFG_NIL;
}

CfgNodeId cfg_preheader_map(CfgInstance cfg, AST_INDEX n)
{
    int i;

    if (!is_loop(n))
    {
	n = out(n);
	if (!is_loop(n)) return CFG_NIL;
    }

    n = gen_get_label(n);

    if (is_null_node(n)) return CFG_NIL;

    i = ast_2_info(cfg, n);

    if (check_cfg_node(cfg, n, i))
	return i;
    else
	return CFG_NIL;
}

static Boolean check_cfg_node(CfgInstance cfg, AST_INDEX node, 
                              CfgNodeId id)
{
    if (cfg->cfgNodes &&
	(id > CFG_NIL) && 
	(id < f_curr_size((Generic) cfg->cfgNodes)) &&
	(CFG_node(cfg, id)->astnode == node))
    {
	return true;
    }
    else
	return false;
}
