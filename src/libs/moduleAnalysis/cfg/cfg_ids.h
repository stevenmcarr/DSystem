/* $Id: cfg_ids.h,v 3.3 1997/03/11 14:35:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- cfg_ids.h
 *
 *        Routines for creating and deleting CFG nodes
 */

#ifndef cfg_new_h
#define cfg_new_h

EXTERN( CfgNodeId, cfg_node_new_id, (CfgInstance cfg));
EXTERN( CfgEdgeId, cfg_edge_new_id, (CfgInstance cfg));
EXTERN( CfgEdgeId, cfg_cd_edge_new_id, (CfgInstance cfg));
EXTERN( void, cfg_node_free, (CfgInstance cfg, CfgNodeId id) );
EXTERN( void, cfg_edge_free, (CfgInstance cfg, CfgEdgeId id) );
EXTERN( void, cfg_cd_edge_free, (CfgInstance cfg, CfgEdgeId id) );

#endif
