/* $Id: cfg_nodes.h,v 3.4 1997/03/11 14:35:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_nodes.h
 *
 *           Include file for CFG nodes builder
 *
 *  CfgInstance cfg_build_nodes(cfgGlobals, root, endAst)
 *      CfgInfo cfgGlobals;
 *      AST_INDEX root, endAst;
 *
 *      build and initialize CFG instance and nodes for the region
 *	predominated by root and postdominated by endAst.
 *	return the CfgInstance.
 */

#ifndef cfg_nodes_h
#define cfg_nodes_h

EXTERN(Boolean, cfg_build_nodes, (CfgInstance cfg, AST_INDEX root));

EXTERN(void, cfg_add_loop_nodes, (CfgInstance cfg));

EXTERN(void, cfg_split_critical_edges, (CfgInstance cfg));

#endif /* !cfg_nodes_h */
