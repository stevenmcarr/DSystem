/* $Id: cfg_edges.h,v 3.2 1997/03/11 14:35:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_edges.h
 *
 *           Include file for CFG edge constructor
 */

#ifndef cfg_edges_h
#define cfg_edges_h

/*
 *  Boolean cfg_build_edges(cfg, root)
 *    CfgInstance cfg;
 *    AST_INDEX root;
 *	build CFG edges for all instances in AST root
 *	assumes CFG nodes already built and initialized
 *
 *	returns true if error (jump to unknown label)
 *
 *
 *  Boolean cfg_is_exec(cfg, node)
 *    CfgInstance cfg;
 *    CfgNodeId node;
 *	return true if any CFG edge into node is "executable"
 *	Assumes CFG construction completed,
 *	but someone else needs to set "executable" flags to something useful.
 *	
 *
 *  Boolean cfg_first_visit(cfg, node)
 *    CfgInstance cfg;
 *    CfgNodeId node;
 *	return true if exactly 1 CFG edge into node is executable
 *	Assumes CFG construction completed,
 *	but someone else needs to set "executable" flags to something useful.
 */

EXTERN(Boolean, cfg_build_edges, (CfgInstance cfg, AST_INDEX root) );
EXTERN(Boolean, cfg_is_exec, (CfgInstance cfg, CfgNodeId node) );
EXTERN(Boolean, cfg_first_time, (CfgInstance cfg, CfgNodeId node) );

#define is_else(root) ((gen_get_node_type(root) == GEN_GUARD) && \
		       (gen_GUARD_get_rvalue(root) == AST_NIL))

#endif /* !cfg_edges_h */
