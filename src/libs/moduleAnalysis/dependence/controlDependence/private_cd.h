/* $Id: private_cd.h,v 1.17 1997/03/11 14:35:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef	private_cd_h
#define	private_cd_h

/************************************************************************/
/*									*/
/*	dep/cd/private_cd.h --						*/
/*	    Control dependence representation, 				*/
/*	    data structures for a control dependence graph.		*/
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
#ifndef	depType_h
#include <libs/moduleAnalysis/dependence/interface/depType.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_header_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif
#ifndef	cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif

#ifndef	strong_h
#include <libs/moduleAnalysis/dependence/utilities/strong.h>
#endif

#ifndef	cd_graph_h
#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
#endif


/* The following information annotates cdNodes in loop distribution.
 */
/* ----------------------- only used in control.c ------------------- */
struct ld_node {
    int		scr;		/* strongly connected region number */
    int		dfn;		/* depth first search number */
    int		run;		/* parallelism (PARALLEL,SEQUENTIAL,PT_NONE) */
    int		tier;		/* earliest time of execution */
    AST_INDEX	newStmts;	/* statements inserted in distribution */
} ;

/* The following information annotates the cdg in loop distribution .
 */
/* ----------------------- only used in control.c ------------------- */
typedef struct ld_desc {
    UtilList **plist;	    /* finest, lexical order, partition node lists */
    UtilList **tlist;	    /* topologically ordered tiered partition lists */
    int	       granularity; /* granularity of tiered partition lists */
    int	       maxtier;     /* depth of execution tree */
    int	       ret;	    /* indicates if distribution can change the loop */
} struct_ldDesc;

/*    Edges --- 
 */
/* ----------------------- only used in cdg_dg.c and control.c -------- */
struct cd_edge {
    cdEdges next_pred;  /* other edges with the same src        */
    cdEdges next_succ;  /* other edges with the same sink       */
    cdNodes src;  	/* the src of this control dependence   */
    cdNodes sink;  	/* the sink of this control dependence  */
    int	    level;	/* the carrier loop level		*/
    int     label;	/* the branch label destination		*/
    int	    index;	/* a numbering of the edges		*/
    int     dgedge;	/* the corresponding edge in dg		*/

} ;

/*    Nodes ---
 */
/* ----------------------- only used in cdg_dg.c and control.c -------- */
struct cd_node	{
    AST_INDEX	stmt;		/* the ast corresponding to this node  */
    int		index;		/* lexical number */
    cdEdges	pred;		/* list of predecessor edges */
    cdEdges	succ;		/* list of successor edges */

    ldNode     *lda;  		/* loop distribution node annotations */
    int		visited;	/* for cdg_walk_nodes */
    
} ;


/* ---------- only used in cdg_dg.c and control.c  and divide.c -------- */
struct cd_graph {
    AST_INDEX  top;       /* if the cdg is connected, this is the root */  
    cdNodes    nodes;	
    cdEdges    edges;	
    int	       ntotal;	  /* total number of nodes */
    int	       noriginal; /* initial number of nodes */
    int	       etotal;	  /* total number of edges */
    int	       newnodes;  /* number of additional nodes available */
    int	       newedges;  /* number of additional edges available */

    ldDesc    *lddesc;	 /* loop distribution information */
    
} ;




#endif	/* private_cd_h */
