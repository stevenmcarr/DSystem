/* $Id: cd_graph.h,v 1.1 1997/03/11 14:35:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: cd_graph.h,v 1.1 1997/03/11 14:35:42 carr Exp $ 
*/

#ifndef cd_graph_h
#define cd_graph_h

/************************************************************************/
/*									*/
/*	dep/cd_graph.h  --						*/
/*	    Control dependence representation, data structures for a    */
/*	    control dependence graph, and exported routine declarations.*/
/*									*/
/************************************************************************/

#ifndef	general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	strong_h
#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#endif

#ifndef	depType_h
#include <libs/moduleAnalysis/dependence/interface/depType.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif
#ifndef	li_instance_h
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#endif
#ifndef	cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif
#ifndef	cd_branch_h
#include <libs/frontEnd/ast/cd_branch.h>
#endif


/**************************************************************************/
/* The CDG structure represents explicitly  both the nodes
 * and the edges in the control dependence graph.  All of the
 * lists are doubley linked for easy access. This makes
 * deletion tricky because nodes and edges may be part of
 * several lists.
 * The cdg also has specialized usages in loop distribution, performance
 * estimation, and the intermediate representation for parallel
 * code generation.
 */


/*	Edges --- 
 */
/* ----------------------- only used in cdg_dg.c and control.c -------- */
struct	cd_edge;

typedef struct cd_edge	cdEdge;
typedef struct cd_edge *cdEdges;


/*	Nodes ---
 */
/* ----------------------- only used in cdg_dg.c and control.c -------- */
struct	cd_node;

typedef struct cd_node	cdNode;
typedef struct cd_node *cdNodes;


/*	Graph ---
 */
/* ---------- only used in cdg_dg.c and control.c  and divide.c -------- */
typedef struct cd_graph	ControlDep;



/* The following information annotates cdNodes in loop distribution.
 */
/* ----------------------- only used in control.c ------------------- */
typedef struct ld_node	ldNode;

/* The following information annotates the cdg in loop distribution .
 */
/* ----------------------- only used in control.c ------------------- */
typedef struct ld_desc	ldDesc;




/************************************************************************/
/*	Entry points for perusing, creating, and understanding		*/
/*	Control Dependences.						*/
/************************************************************************/

/* --------------------------------------------------------------------	*/
/* These entry points are in	/rn/src/dep/cd/cdg_cd.c
 */
EXTERN( ControlDep *,  dstr_cd_graph,
		(DG_Instance * dg, LI_Instance * li, SideInfo * infoPtr,
		 Adj_List * adj_list, AST_INDEX loop ) );
/* Builds a control dependence graph for a loop and maps it to
 * a partition specified in adj.
 */


EXTERN( AST_INDEX, cd_insert_evar,
		(DG_Instance * dg, SideInfo * infoPtr,
		 ControlDep * cd, int p, int first, int n, AST_INDEX loop ) );
    

EXTERN( Boolean, dstr_cd_restructure,
		(DG_Instance * dg, LI_Instance * li, SideInfo * infoPtr,
		 ControlDep * cd, Adj_List * adj_list, AST_INDEX loop ) );


EXTERN( AST_INDEX, dstr_cd_rebuild_tree, 
		(LI_Instance * li, ControlDep * cd, 
		 Adj_List * adj_list, AST_INDEX old_do, int level ) );


/*  These entry points are in /rn/src/libs/ft_analysis/dep/cfg_dgcd.c
 */
EXTERN(void, dg_build_module_cds,
       		(DG_Instance * dg, SideInfo * infoPtr, CfgInfo cfgGlobals));
/* 
 * Takes a control flow graph and builds the control dependences.
 * The control dependences are inserted into ped's dg, the 
 * dependence graph.
 */ 

EXTERN(void, dg_build_subprogram_cds, 
		(DG_Instance * dg, SideInfo * infoPtr, CfgInstance cfg));
/*
 *  Takes a cfg instance (i.e., cfg for one subprogram or fragment)
 *  and builds the control dependences, adding them to ped's dg.
 */


/* --------------------------------------------------------------------	*/
/* These entry points are in /rn/src/ped_cp/dg/cdg_dg.c.
 */

EXTERN(ControlDep *, dg_build_cdg, 
		(DG_Instance * dg, SideInfo * infoPtr, 
		 AST_INDEX root, double extra));
/* Extracts the control dependences in ped's dg for the subprogram
 * rooted at root.  A root may be for a module, a subroutine, a loop, etc.
 * The control dependences are placed in a graph by themselves which is
 * returned.  The graph is cyclic.  It is also rooted, but only if all the 
 * executable statements in the module are reachable.  Extra multiplies
 * the number of edges and nodes in the cdg, and that additional number
 * of nodes and edges is also allocated.  The default for extra is 1.
 */

EXTERN(Boolean, dg_delete_cds, 
		(DG_Instance * dg, SideInfo * infoPtr, 
		 AST_INDEX root, Boolean check));
/* Starts at root and deletes control dependences for every statement
 * in root's scope.  If check is true, it returns false if there are 
 * edges into or out of the scope that are not deleted, otherwise it
 * returns true.
 */

EXTERN(void, cdg_free, 	(ControlDep *cd));
/* Frees a control dependence graph and its substructures.
 */

EXTERN(void, cdg_delete_edge, (cdEdge *edge, cdEdge *prev));
/* Deletes edge out of a ControlDep graph
 */

typedef FUNCTION_POINTER(int, cdg_action_callback,
 (cdNode *node, Generic parm));

EXTERN(int, cdg_walk_nodes, 
		(ControlDep *cdg, cdNode *node, cdg_action_callback pre_action,
                 cdg_action_callback post_action, Generic parm));
/* Walks a control dependence graph starting at node in depth first
 * order, applying pre_action before visiting a node and post_action
 * after visiting a node.  Both are called with parm. It is modelled
 * after walk_statements in el/walk.c.
 */


EXTERN(void, cdg_print, (ControlDep *cd));

#endif /* cd_graph_h */
