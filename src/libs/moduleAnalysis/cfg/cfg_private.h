/* $Id: cfg_private.h,v 3.12 1997/03/11 14:35:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* 
 *
 * -- cfg_private.h
 *
 *	   Private include file for Control Flow Graph builders
 */

#ifndef __cfg_private_h__
#define __cfg_private_h__

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef short SmallBool;
typedef short SmallInt;
typedef char  TinyBool;
typedef short TinyInt;


#define CFG_INIT_SIZE 0x80

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/cfg/cfg_labels.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/arrays/ExtensibleArray.h>
#include <libs/support/arrays/ExtendingArray.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/moduleAnalysis/cfg/tree_util.h>
#include <stdlib.h>
#include <string.h>

#define SSA_WORK_SLOTS 7
#define VAL_WORK_SLOTS 7

typedef struct cfg_generic_list_struct {
    Generic stuff;
    struct cfg_generic_list_struct *next;
} *CfgList;


/*
 *  control dependence edge in subprogram CFG
 */
typedef struct {
    CfgNodeId src;		/* source node */
    CfgNodeId dest;		/* destination node */
    CfgEdgeId inNext;		/* next edge in "in" list of dest node */
    CfgEdgeId outNext;		/* next edge in "out" list of src node */
    SmallInt label;		/* number indicating which out-edge this is */
    TinyInt level;		/* carrying level of loop */
    TinyBool freed;		/* 0 if in use */
} CfgCdEdge;


/*
 *  edge in subprogram CFG
 */
typedef struct {
    CfgNodeId src;		/* source node */
    CfgNodeId dest;		/* destination node */
    CfgEdgeId inNext;		/* next edge in "in" list of dest node */
    CfgEdgeId outNext;		/* next edge in "out" list of src node */
    SmallInt label;		/* number indicating which out-edge this is */
    TinyBool exec;		/* executable flag */
    TinyBool freed;		/* 0 if in use */
} CfgEdge;


/*
 *  node in subprogram CFG -- roughly correspond to executable
 *  or expression-containing statements in AST
 */
typedef struct {
    AST_INDEX astnode;		/* ast node # of statement */
    int lbldef;			/* ast stmt label, if exists */
    CfgEdgeId ins;		/* CFG in-edges from predecessors */
    CfgEdgeId outs;		/* CFG out-edges to successors */
    CfgEdgeId cdIns;		/* incoming control dependences */
    CfgEdgeId cdOuts;		/* outgoing control dependences */
    Generic ssaKids;		/* list of references associated with stmt */
    SmallInt fanIn;		/* CFG edge fan in */
    SmallInt fanOut;		/* CFG edge fan out */
    TinyBool freed;		/* 0 if in use */
} CfgNode;


/*
 *  CfgInstance - subroutine, function, program, block data, etc.
 */
/* typedef */
struct cfg_instance_struct {
    CfgInstanceType type;   /* program, subroutine, function, fragment, etc. */
    STR_INDEX name;	    /* symtab index of subprogram name */
    CfgInfo cfgGlobals;	    /* point back to global CfgInfo structure */
    AST_INDEX astnode;	    /* ast node # of module root or subprogram root */
    AST_INDEX endAst;	    /* ast node # for end of unstructured fragment */
    SymDescriptor symtab;   /* Fortran symbol table handle */
    LabelMap lblMap;	    /* map from label # to node # */
    CfgNode *cfgNodes;	    /* f_array for the cfg nodes */
    CfgEdge *cfgEdges;	    /* f_array for the cfg edges */
    CfgCdEdge *cfgCdEdges;  /* f_array for the cfg control dependence edges */
    CfgNodeId* topMap;	    /* Topologically sorted CFG map */
    CfgNodeId start;	    /* id of start node of CfgInstance */
    CfgNodeId end;	    /* " " " end " " " " ". Global inst has none */
    DomTree predom;	    /* (xarray) predominator tree for cfg nodes */
    DomTree postdom;	    /* (xarray) postdominator tree for cfg nodes */
    TarjTree intervals;	    /* (xarray) Tarjan interval tree for cfg nodes */
    AST_INDEX* comnAstList; /* Vector of COMMON and TASK_COMMON statements */
    struct cfg_instance_struct *next;
    struct cfg_instance_struct *prev;

    /*
     *	Space for SSA and value numbering stuff
     */
    Generic ssaStuff[SSA_WORK_SLOTS];
    Generic valStuff[VAL_WORK_SLOTS];

} ; /* *CfgInstance */

#define CFG_node(cfg, id)	(&((cfg)->cfgNodes[id]))
#define CFG_edge(cfg, id)	(&((cfg)->cfgEdges[id]))
#define CFG_cdedge(cfg, id)	(&((cfg)->cfgCdEdges[id]))

/*
 *  Store callback functions for a client.
 */
typedef struct cfg_client_struct {
    Generic	handle;			/* Passed as first arg to each fn    */
					/* Must match for Register/Deregister*/
    CFG_INSTANCE_FN	create_instance;
    CFG_INSTANCE_FN	destroy_instance;
    CFG_INSTANCE_FN	dump_instance;
    /*
     *	Create and destroy functions for nodes and edges are *not* called
     *	when whole CfgInstances are created and destroyed -- only when there
     *	are update operations on individual nodes and edges.
     */
    CFG_NODE_FN		create_node;
    CFG_NODE_FN		destroy_node;
    CFG_NODE_FN		dump_node;
    CFG_EDGE_FN		create_edge;
    CFG_EDGE_FN		destroy_edge;
    CFG_EDGE_FN		dump_edge;

    struct cfg_client_struct *next;
} *CfgClient;

/*
 *  put all the other structures into a super structure that will be passed
 *  around everywhere
 */
/* typedef */
struct cfg_info_struct {
    FortTree	ft;		/* the module */
    Generic*	sideArray;
    CfgInstance	firstInst, lastInst;
    CfgClient	firstClient;	/* registration of callback functions*/
    Generic	ssaParms;
    Generic	valParms;
}; /* *CfgInfo */

#include <libs/moduleAnalysis/cfg/cfg_info.h>

EXTERN(void, cfg_free_all_cds, (CfgInstance cfg));
	/* 
	 *  Removes all control dependences while not changing the rest
	 *  of the CFG.
	 */

#endif /* !__cfg_private_h__ */
