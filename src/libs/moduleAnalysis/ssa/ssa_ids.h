/* $Id: ssa_ids.h,v 3.4 1997/03/11 14:36:13 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_ids.h
 *
 */
#ifndef ssa_ids_h
#define ssa_ids_h

EXTERN(SsaNodeId, ssa_node_new_id, (CfgInstance cfg, SsaType type));
EXTERN(SsaNodeId, ssa_edge_new_id, (CfgInstance cfg));
EXTERN(void, ssa_node_free, (CfgInstance cfg, SsaNodeId id));
EXTERN(void, ssa_edge_free, (CfgInstance cfg, SsaEdgeId id));
EXTERN(void, ssa_KillKids,
		(Generic junk, CfgInstance cfg, CfgNodeId id));

EXTERN(void, ssa_init_maps, (CfgInstance cfg));
EXTERN(void, ssa_kill_maps, (CfgInstance cfg));
EXTERN(void, ssa_node_put_map, (CfgInstance cfg, AST_INDEX ast, SsaNodeId id));
EXTERN(void, ssa_node_zap_map, (CfgInstance cfg, AST_INDEX ast));

#endif /* !ssa_ids_h */
