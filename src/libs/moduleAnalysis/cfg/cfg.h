/* $Id: cfg.h,v 3.13 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*****************************************************************************
 *
 *
 *  -- cfg.h
 *
 *  Public header file for AST-based Control Flow Graph package.
 *
 *****************************************************************************/

#ifndef cfg_h
#define cfg_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif
#ifndef cd_branch_h
#include <libs/frontEnd/ast/cd_branch.h>
#endif

/*
 *  Global information -- for all subprograms in a module/compilation unit
 */
struct cfg_info_struct;
typedef struct cfg_info_struct *CfgInfo;

/*
 *  Local information -- for a particular subprogram
 */
struct cfg_instance_struct;
typedef struct cfg_instance_struct *CfgInstance;


typedef Generic CfgEdgeId;
typedef Generic CfgNodeId;
typedef CfgEdgeId CfgCdId;
#define CFG_NIL (-1)
#define CFG_NO_LABEL (0)

/*
 *  Typedefs for callback functions
 */
typedef FUNCTION_POINTER(void, CFG_INSTANCE_FN,
			 (Generic handle, CfgInstance cfg));
typedef FUNCTION_POINTER(void, CFG_NODE_FN,
			 (Generic handle, CfgInstance cfg, CfgNodeId node));
typedef FUNCTION_POINTER(void, CFG_EDGE_FN,
			 (Generic handle, CfgInstance cfg, CfgEdgeId edge));

/*
 *  CfgInstance types
 */
typedef enum {
    CFG_GLOBAL,		/* The global instance for file -- no graph built */
    CFG_PROGRAM,	/* Main program */
    CFG_FUNCTION,	/* External function */
    CFG_SUBROUTINE,	/* External subroutine */
    CFG_BLOCK_DATA,	/* Block data -- contains common block data stmts */
    CFG_FRAGMENT	/* Non-procedural */
} CfgInstanceType;




/*****************************************************************************
 *
 *  CONTROL FLOW GRAPH INTERFACE ROUTINES
 *
 *****************************************************************************/




/*****************************************************************************
 *
 *  CREATION and DESTRUCTION
 *
 *  There are two kinds of CFG object:
 *
 *	CfgInfo		Global CFG data for the module
 *	CfgInstance	CFG data for a subprogram
 *
 *  This file always uses "cfgGlobals" for a CfgInfo,
 *  and "cfg" for a CfgInstance.
 *
 *  Registry gives functions for the CFG stuff to call on creation/destruction
 *  of objects.
 *
 *  Creation functions are called after the object has been created.
 *  Destruction functions are called before the object is destroyed.
 *  Dump functions are called after the object has been printed.
 *
 *  Node and edge creation/destruction functions are not called
 *  when a whole instance is destroyed/created, only when individual nodes
 *  or edges are manipulated.
 *
 *  If a client registers after CfgInstances have been built for a
 *  given CfgGlobals, then the create_instance function will be called 
 *  for each pre-existing CfgInstance.
 */

EXTERN(CfgInfo, cfg_Open,
		(FortTree ft));

EXTERN(void, cfg_Close,
		(CfgInfo cfgGlobals));

EXTERN(AST_INDEX, cfg_get_global_root, (CfgInfo cfgGlobals));
    /*
     *  Returns the AST root of the current module
     */

EXTERN(void, cfg_Register,
		(CfgInfo cfgGlobals,
		 Generic handle,	/* pass-back handle to the client    */

		 CFG_INSTANCE_FN	create_instance,
		 CFG_INSTANCE_FN	destroy_instance,
		 CFG_INSTANCE_FN	dump_instance,
		 CFG_NODE_FN		create_node,
		 CFG_NODE_FN		destroy_node,
		 CFG_NODE_FN		dump_node,
		 CFG_EDGE_FN		create_edge,
		 CFG_EDGE_FN		destroy_edge,
		 CFG_EDGE_FN		dump_edge));

EXTERN(void, cfg_Deregister,
		(CfgInfo cfgGlobals, Generic handle));


/*
 *  Change notification (in cfg_changes.c)
 *
 *  CfgInstances are not guaranteed to persist across calls to
 *	these routines.	 Be sure to re-acquire your instance handle.
 */
EXTERN(void, cfg_TreeWillChange, (CfgInfo cfgGlobals, AST_INDEX node));
EXTERN(void, cfg_TreeChanged, (CfgInfo cfgGlobals, AST_INDEX node));



/****************************************************************************
 *
 *  MANIPULATING SUBPROGRAM CfgInstances (in cfg_main.c and cfg_utils.c)
 *
 *  cfg_build_inst
 *	Constructs an instance for the root passed.  The subprogram
 *	instance list will be checked for an instance including the
 *	root -- if that instance is for the root, with the same endAst,
 *	it will be returned, else the old instance will be thrown away
 *	to prevent corruption, and a new instance for this root/endAst
 *	built.
 *
 *	Instances whose roots are subprograms are saved with the cfgGlobals.
 *
 *  cfg_get_inst
 *	Tries to find an existing CfgInstance for that subprogram
 *	name, else gets one from cfg_build_inst.
 *
 *  cfg_get_first_inst
 *	Returns an instance for the first subprogram in the module,
 *	building it if necessary.
 *
 *  cfg_get_next_inst
 *	Returns an instance for the next subprogram in the module,
 *	building it if necessary.
 */

EXTERN(CfgInstance, cfg_build_inst,
		(CfgInfo cfgGlobals, AST_INDEX root, AST_INDEX endAst));

EXTERN(void, cfg_destroy_inst, (CfgInstance cfg));

EXTERN(CfgInstance, cfg_get_inst, (CfgInfo cfgGlobals, char *name));
	/*
	 *  cfg for the named subprogram in cfgGlobals' module
	 */

EXTERN(CfgInstance, cfg_get_first_inst, (CfgInfo cfgGlobals));
	/*
	 *  Start a walk through the instances; use cfg_get_next_inst
	 *  to continue.
	 *
	 *  This cfg instance is guaranteed to be the "CFG_GLOBAL"
	 *  instance if one exists.
	 */

EXTERN(CfgInstance, cfg_get_next_inst, (CfgInstance cfg));
	/*
	 *  Return the next instance in occurrence order in the module.
	 */

EXTERN(CfgInstanceType,	cfg_get_inst_type, (CfgInstance cfg));
	/*
	 *  Return type, according to enumerated type defined above.
	 */

/****************************************************************************
 *
 *  MAP FROM CFG->AST and AST->CFG
 *
 *  routines in cfg_utils.c and cfg_ssa_info.c
 */

EXTERN(AST_INDEX, cfg_node_to_ast, (CfgInstance cfg, CfgNodeId n));
	/*
	 *  From CFG node (a statement) to corresponding AST node.
	 *	Note that there is no CFG node for an IF, just for its GUARD.
	 */

EXTERN(CfgNodeId, cfg_node_from_ast, (CfgInstance cfg, AST_INDEX a));
	/*
	 *  From AST node (a statement) to corresponding CFG node.
	 *	Note that there is no CFG node for an IF, just for its GUARD.
	 */

/*
 *  DO-loops require special functions... (in cfg_utils.c)
 *	in order to construct edges correctly, an extra CFG node is built
 *	for the DO ast node 
 *		-- the "header" is the sink of normal backedges
 *			and the source of normal exit
 *
 *		-- the "preheader" is the sink of explicit jumps to the 
 *			DO statement.
 */
EXTERN(CfgNodeId, cfg_header_from_ast, (CfgInstance cfg, AST_INDEX a));

EXTERN(CfgNodeId, cfg_preheader_from_ast, (CfgInstance cfg, AST_INDEX a));

/*
 *  Is there a CFG node associated with the AST node?
 */
#define cfg_node_exists_for_ast(cfg, n) \
    (cfg_node_from_ast(cfg, n) != CFG_NIL)





/****************************************************************************
 *
 *  MANIPULATING CFG NODES
 *
 */

EXTERN(CfgNodeId, cfg_start_node, (CfgInstance cfg));
	/*
	 *  Returns the start node of the current CFG instance,
	 *  also the root of the PREdominator and interval trees.
	 */

EXTERN(CfgNodeId, cfg_end_node,   (CfgInstance cfg));
	/*
	 *  Returns the end node of the current CFG instance
	 *  (has no corresponding AST node).  Also the root of the
	 *  POSTdominator tree.
	 */

EXTERN(CfgNodeId, cfg_get_first_node, (CfgInstance cfg));
	/*
	 *  Returns an arbitray CfgNodeId, such that 
	 *  cfg_get_next_node will cycle through the remaining nodes.
	 */

EXTERN(CfgNodeId, cfg_get_next_node,  (CfgInstance cfg, CfgNodeId node));
	/*
	 *  Returns the "next" cfg node after the one passed.
	 *  This ordering should be treated as completely arbitrary.
	 *  Returns CFG_NIL if there are no more nodes.
	 */

EXTERN(CfgNodeId, cfg_get_last_node,  (CfgInstance cfg));
	/*
	 *  Returns an arbitray CfgNodeId, such that 
	 *  cfg_get_prev_node will cycle through the remaining nodes.
	 */

EXTERN(CfgNodeId, cfg_get_prev_node,  (CfgInstance cfg, CfgNodeId node));
	/*
	 *  Returns the "prev" cfg node after the one passed.
	 *  This ordering should be treated as completely arbitrary.
	 *  Returns CFG_NIL if there are no more nodes.
	 *
	 *  Only guarantee is that this order is opposite of 
	 *  cfg_get_first_node, cfg_get_next_node order.
	 */

EXTERN(CfgNodeId, cfg_node_max, (CfgInstance cfg));
	 /*
	  *  Returns a CfgNodeId greater than any actually allocated.
	  */

EXTERN(CfgEdgeId, cfg_edge_max, (CfgInstance cfg));
	 /*
	  *  Returns a CfgEdgeId greater than any actually allocated.
	  */



/****************************************************************************
 *
 *  DOMINATOR TREES -- dtree.c
 *
 *	PREdominator and POSTdominator trees are accessed via CfgNodeId's.
 */

/*
 *  Dominator information (pre, post) pointer type.
 */
struct dominator_entry_struct;
typedef struct dominator_entry_struct *DomTree;

EXTERN(DomTree,	cfg_get_predom,  (CfgInstance cfg));
EXTERN(DomTree,	cfg_get_postdom, (CfgInstance cfg));
	/*
	 *  -- cfg_get_predom, cfg_get_postdom
	 *
	 *  Return a pointer to the (pre, post)dominator relation
	 *  for the passed CFG instance. 
	 */

EXTERN(CfgNodeId, dom_idom, (DomTree dom, CfgNodeId node));
	/*
	 *  Immediate dominator.
	 */

EXTERN(CfgNodeId, dom_kids, (DomTree dom, CfgNodeId node));
	/*
	 *  First dominatee.
	 */

EXTERN(CfgNodeId, dom_next, (DomTree dom, CfgNodeId node));
	/*
	 *  Next dominatee with same dominator.
	 */

EXTERN(Boolean,	  dom_is_dom, (DomTree dom, CfgNodeId a, CfgNodeId b));
	/*
	 *  Return true if "a" (possibly transitively) dominates "b".
	 */

EXTERN(CfgNodeId, dom_lca,    (DomTree dom, CfgNodeId a, CfgNodeId b));
	/*
	 *  Return the least common ancestor of "a" and "b" in "dom".
	 */

EXTERN(void,	  dom_print,  (DomTree dom, CfgNodeId root));
	/*
	 *  Print the dominator tree starting at "root".
	 */

EXTERN(Boolean,	  cfg_is_reachable, (CfgInstance cfg, CfgNodeId n));
EXTERN(Boolean,	  cfg_reaches_end,  (CfgInstance cfg, CfgNodeId n));
	/*
	 *  These return true if there is a path to the node from start,
	 *  or to end from the node, respectively.
	 */






/****************************************************************************
 *
 *  TARJAN INTERVAL TREE -- tarjan.c
 *
 *	Tree of natural loop structur of the program.
 */

/*
 *  Tarjan interval information (loops)
 */
struct tarjan_entry_struct;
typedef struct tarjan_entry_struct *TarjTree;

/*
 *  Acyclic	is single-statement, acyclic (trivial) interval
 *  Interval	is true reducible Tarjan interval (natural loop)
 *  Irreducible is not a true interval -- multi-entry loop
 */
typedef enum {
    TARJ_NOTHING,
    TARJ_ACYCLIC,
    TARJ_INTERVAL, 
    TARJ_IRREDUCIBLE
    } TarjType;

/*
 *  Types of CFG edges, w.r.t. loop structure
 */
typedef enum {
    TARJ_NORMAL,
    TARJ_LOOP_ENTRY,
    TARJ_IRRED_ENTRY,
    TARJ_ITERATE
    } TarjEdgeType;

EXTERN(TarjTree, cfg_get_intervals, (CfgInstance cfg));
	/*
	 *  -- cfg_get_intervals
	 *
	 *  Return a pointer to the Tarjan interval structure
	 *  for the passed CFG instance.
	 *  This is a tree indicating the nesting of natural loops.
	 */

EXTERN(void, tarj_sort,	(CfgInstance cfg));
    /*
     *  Sort the Tarjan interval tree according to topological ordering...
     *  determined by ignoring backedges.
     */

EXTERN(TarjType, tarj_type, (TarjTree tarjans, CfgNodeId id));

EXTERN(int,	 tarj_level, (TarjTree tarjans, CfgNodeId id));
	/*
	 *  Nesting level, counting out-to-in
	 */

EXTERN(CfgNodeId, tarj_outer, (TarjTree tarjans, CfgNodeId id));
	/*
	 *  Loop immediately surrounding this node or loop
	 */

EXTERN(CfgNodeId, tarj_inners, (TarjTree tarjans, CfgNodeId id));
	/*
	 *  A statement immediately inside this loop.
	 */

EXTERN(CfgNodeId, tarj_next,   (TarjTree tarjans, CfgNodeId id));
	/*
	 *  Another statement with same immediate surrounding loop.
	 */

EXTERN(CfgNodeId, tarj_inners_last, (TarjTree tarjans, CfgNodeId id));
	/*
	 *  The last-walked statement nested under this loop header
	 */

EXTERN(Boolean, tarj_is_header, (TarjTree tarjans, CfgNodeId id));
        /*
	 *  Is it a header node ?
	 */

EXTERN(Boolean, tarj_is_first, (TarjTree tarjans, CfgNodeId id));
        /*
	 *  Is it the first node of its interval?
	 */

EXTERN(Boolean, tarj_is_last, (TarjTree tarjans, CfgNodeId id));
        /*
	 *  Is it the lsst node of its immediate surrounding interval ?
	 *  (i.e., are there no more nodes at this level?)
	 */

EXTERN(Boolean,	tarj_contains, (TarjTree tarjans, CfgNodeId a, CfgNodeId b));
	/*
	 *  Returns true if "b" is inside loop "a" (maybe several levels deep)
	 */

EXTERN(int, tarj_exits, (TarjTree tarjans, CfgNodeId src, CfgNodeId dest));
	/*
	 *  Returns the number of loop levels exited going from src to dest
	 *  (doesn't have to be a CfgEdge between them).
	 */

EXTERN(CfgNodeId, tarj_loop_exited, 
	     (TarjTree tarjans, CfgNodeId src, CfgNodeId dest));
	/*
	 *  Returns the outermost loop exited in going from src to dest
	 *  (doesn't have to be a CfgEdge between them).
	 */

EXTERN(TarjEdgeType, tarj_edge_type, 
		(TarjTree tarjans, CfgNodeId src, CfgNodeId dest));
	/*
	 *  Returns the edge type of a possible edge from src to dest; gives
	 *  information about whether or not a loop is entered, iterated, etc.
	 */

EXTERN(CfgNodeId, tarj_lca, (TarjTree tarjans, CfgNodeId a, CfgNodeId b));
	/*
	 *  Gives the innermost loop containing both a and b
	 */

EXTERN(void, tarj_print, (TarjTree tarj, CfgNodeId node));
	/*
	 *  Prints the tarjan interval tree starting at node.
	 */

EXTERN(Boolean, cfg_is_backedge, (CfgInstance cfg, CfgEdgeId edgeId));
	/*
	 *  Returns TRUE if the CFG edge is an back edge.
	 */

EXTERN(int, cfg_get_forward_fanout, (CfgInstance cfg, CfgNodeId cfgId));
	/*
	 *  Returns the number of forward CFG out edges
	 */

EXTERN(int, cfg_get_forward_fanin,  (CfgInstance cfg, CfgNodeId cfgId));
	/*
	 *  Returns the number of forward CFG in edges
	 */

EXTERN(CfgNodeId, cfg_containing_loop, (CfgInstance cfg, AST_INDEX an));
	/*
	 *  Shortcut to get innermost loop surrounding the ast node.
	 *  If the AST_INDEX is for a loop header statement, return
	 *  the header of the next outer surrounding loop.
	 */

/*********************************************************************
 *
 * Interval Tree Iterators
 *
 * Given the following Tarjan interval tree:
 * <node id>(level,type)       //  Original program
 *  0(0,Acyclic)
 *     1(0,Acyclic)            //  program example
 *     2(1,Interval)           //  do i = 1, n
 *        3(1,Acyclic)         //     dummy = 0
 *        4(1,Acyclic)         //     dummy = 0
 *     10(0,Acyclic)           //  enddo
 *     5(1,Interval)           //  do i = 1, n
 *        6(1,Acyclic)         //     dummy = 0
 *        7(2,Interval)        //     do j = 1, n
 *           8(2,Acyclic)      //        dummy = 0
 *        11(1,Acyclic)        //     enddo
 *     9(0,Acyclic)            //  enddo
 *
 * The order in which the iterators will return <node id>'s is for
 * PreOrder:         0 1 2 3 4 10 5 6 7 8 11 9  [top-down, forward]
 * PostOrder:        1 3 4 2 10 6 8 7 11 5 9 0  [bottom-up, forward]
 * ReversePreOrder:  9 11 8 7 6 5 10 4 3 2 1 0  [bottom-up, backward]
 * ReversePostOrder: 0 9 5 11 7 8 6 10 2 4 3 1  [top-down, backward]
 *
 * Usage:
 * for (CfgNodeId cn = cfg_tarj_first(cfg, order);
 *      cn != CFG_NIL;
 *      cn = cfg_tarj_next(cfg, cn, order) { ... }
 */

#define cfg_tarj_first(cfg, order) (cfg_tarj_next(cfg, CFG_NIL, order))
EXTERN(CfgNodeId, cfg_tarj_next, (CfgInstance cfg, CfgNodeId cn,
				  TraversalOrder order));


/****************************************************************************
 *
 *  OTHER DERIVED INFORMATION 
 *
 *	Topological order map (constructed ignoring backedges).
 */


/*
 *  Returns a pointer to a map of a topological order on the CfgNodeIds.
 *  cfg_get_first_node, cfg_get_next_node can be used to walk through
 *  the map in the forward direction.
 *
 *  To go through the map backwards... use cfg_get_last_node, 
 *  cfg_get_prev_node.
 *
 *  For example, if we do 
 *		map = cfg_top_sort(cfg);
 *  then
 *		map[cfg_get_first_node] is the topologically first node
 *  and
 *		map[cfg_get_last_node] is the topologically last node
 *
 *  This function needs Tarjan intervals and will create them
 *  if they do not exist.
 */
EXTERN(CfgNodeId *, cfg_top_sort,  (CfgInstance cfg));





/****************************************************************************
 *
 *  MANIPULATING CFG EDGES
 *
 */

EXTERN(CfgEdgeId,   cfg_first_from_ast,(CfgInstance cfg, AST_INDEX a));
EXTERN(CfgEdgeId,   cfg_first_from_cfg,(CfgInstance cfg, CfgNodeId n));
EXTERN(CfgEdgeId,   cfg_first_to_ast,  (CfgInstance cfg, AST_INDEX a));
EXTERN(CfgEdgeId,   cfg_first_to_cfg,  (CfgInstance cfg, CfgNodeId n));

EXTERN(CfgEdgeId,   cfg_next_from,     (CfgInstance cfg, CfgEdgeId e));
EXTERN(CfgEdgeId,   cfg_next_to,       (CfgInstance cfg, CfgEdgeId e));

EXTERN(CfgNodeId,   cfg_edge_src,      (CfgInstance cfg, CfgEdgeId e));
EXTERN(CfgNodeId,   cfg_edge_dest,     (CfgInstance cfg, CfgEdgeId e));
EXTERN(int,	    cfg_edge_label,    (CfgInstance cfg, CfgEdgeId e));
    /*
     *	CFG edges are labeled as described in <cd.h>.
     */

EXTERN(int,	    cfg_node_fanin,    (CfgInstance cfg, CfgNodeId n));
EXTERN(int,	    cfg_node_fanout,   (CfgInstance cfg, CfgNodeId n));



/****************************************************************************
 *
 *  MANIPULATING CFG-CD EDGES (control dependences)
 *
 *  These are built when you first request control dependences for 
 *  a particular cfg node (but not rebuilt unless the cfg is changed).
 */
EXTERN(CfgCdId,	  cfg_cd_first_from,	(CfgInstance cfg, CfgNodeId n));
EXTERN(CfgCdId,	  cfg_cd_first_to,	(CfgInstance cfg, CfgNodeId n));

EXTERN(CfgCdId,	  cfg_cd_next_from,	(CfgInstance cfg, CfgCdId e));
EXTERN(CfgCdId,	  cfg_cd_next_to,	(CfgInstance cfg, CfgCdId e));

EXTERN(CfgNodeId, cfg_cd_src,		(CfgInstance cfg, CfgCdId e));
EXTERN(CfgCdId,	  cfg_cd_dest,		(CfgInstance cfg, CfgCdId e));
EXTERN(int,	  cfg_cd_label,		(CfgInstance cfg, CfgCdId e));
EXTERN(int,	  cfg_cd_level,		(CfgInstance cfg, CfgCdId e));

/* support a deprecated initialization routine */
#define cfg_build_cfg_cds(cfg)	((void)cfg_cd_first_from(cfg, \
							 cfg_start_node(cfg)))


/*
 *  Debugging/print routines
 */
EXTERN(void,	cfg_dump,		(CfgInstance cfg));
EXTERN(void,    cfg_stats,              (CfgInstance cfg));
EXTERN(void,	cfg_sorted_dump,	(CfgInstance cfg));
    /*
     *	latter version sorts according to topological order,
     *	ignoring loop backedges
     */

EXTERN(SymDescriptor, cfg_get_inst_symtab, (CfgInstance cfg));
EXTERN(AST_INDEX, cfg_get_inst_root, (CfgInstance cfg));
EXTERN(char*, cfg_get_inst_name, (CfgInstance cfg));
EXTERN(CfgInfo, cfg_get_globals, (CfgInstance cfg));

EXTERN(CdBranchType, cfg_branch_type, (CfgInstance cfg, CfgNodeId n));
	/*
	 *  Classifies the type of branch node; if fanout <= 1,
	 *  returns CD_UNCONDITIONAL.
	 */

#endif /* ifndef cfg_h */
