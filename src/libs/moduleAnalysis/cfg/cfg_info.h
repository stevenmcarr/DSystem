/* $Id: cfg_info.h,v 3.3 1997/03/11 14:35:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 *  -- cfg_info.h
 *
 *              -- Paul Havlak
 */

#ifndef __cfg_info_h__
#define __cfg_info_h__

typedef enum {
    CFG_INFO_NODE,
    CFG_INFO_EDGE,
    SSA_INFO_NODE,
    SSA_INFO_EDGE
} CfgInfoType;

#define cfg_node_put_map(cfg, ast, id) \
    cfg_put_map(cfg, ast, (Generic) id, CFG_INFO_NODE)

#define cfg_edge_put_map(cfg, ast, id) \
    cfg_put_map(cfg, ast, (Generic) id, CFG_INFO_EDGE)

#define cfg_node_map(cfg, ast)	\
    ((CfgNodeId)cfg_map(cfg, ast, CFG_INFO_NODE))

#define cfg_edge_map(cfg, ast)	\
    ((CfgEdgeId)cfg_map(cfg, ast, CFG_INFO_EDGE))

/*
 *  Add an info item for an AST_INDEX
 */
EXTERN(void, cfg_put_map,
		(CfgInstance cfg, AST_INDEX a, int i, CfgInfoType type));

/*
 *  From AST node (a statement) to correspoinding CFG/SSA object.
 *	Note that there is no CFG node for an IF, just for its GUARD.
 */
EXTERN(Generic, cfg_map, (CfgInstance cfg, AST_INDEX a,
				  CfgInfoType type));

/*
 *  DO-loops require special functions... (in cfg_info.c)
 *	in order to construct edges correctly, an extra CFG node is built
 *	for the DO ast node (actually hung off its label)
 *
 *		-- the "header" is the sink of normal backedges
 *			and the source of normal exit
 *
 *		-- the "preheader" is the sink of explicit jumps to the 
 *			DO statement.
 */
EXTERN(CfgNodeId, cfg_header_map, (CfgInstance cfg, AST_INDEX a));
EXTERN(CfgNodeId, cfg_preheader_map, (CfgInstance cfg, AST_INDEX a));

/*
 *  Is there a CFG node associated with the AST node?
 */
#define cfg_node_exists(cfg, n) (cfg_node_map(cfg, n) != CFG_NIL)

/*
 *  This is always false
 */
#define cfg_edge_exists(cfg, n) (cfg_edge_map(cfg, n) != CFG_NIL)

#endif
