/* $Id: private_el.h,v 1.5 1997/03/11 14:35:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef	private_el_h
#define	private_el_h
/***************************************************************************

   NEW: Views on top of the dependence filter facility (leads to a pretty
   sorry user interface, but at least a decent functionality).
   kats 9/90

   Data structures to implement the Edge List abstraction. This is used by
   the editor to browse through the list of dependences in the source file, 
   and by the abstractions that implement parallel transformations.
   - Vas, Sept 1987.
 
   NOTE: As long as the dependence info used by ParaScope is that generated 
   by PSERVE, remember to convert variable names into upper case before
   making any comparisons using strcmp(). This will be unnecessary when
   ParaScope becomes completely independent of PSERVE. 
   -Vas, May 1988.
 ****************************************************************************/

#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>


/*		S T R U C T U R E S    F O R    EL_INSTANCE		*/


struct edge_list {
    EDGE_INDEX	edge_index; 	/* index of dep edge in the DG abstraction */
    Boolean     active;
    AST_INDEX	def_before;
    AST_INDEX	use_after;
    int         why;
    Boolean	user;
    char	*cblock;
    int		dims;
} ;

struct dep_query {    /* structure to hold the query */
    char	*type;	
    AST_INDEX	 src;
    AST_INDEX	 sink;
    char	*name;
    char	*dims;
    char	*block;
} ;

struct el_instance {
    Edge_List	*edgelist;          	/* list of dependences for selected region */
    ControlDep  *cd;			/* control dependence graph for the loop, kats 9/90  */
    int		current_dep_num;    
    int		total_num_of_deps;  		/* kats 9/90 all this lc,li, etc junk */
    int		num_lc;			/* loop carried */
    int 	num_lc_shared;		/* loop carried on shared variables */
    Boolean	lc;			/* if loop carried deps are to be shown */
    Boolean	control;		/* if control deps are to be shown */
    Boolean	li;			/* if loop independent deps are to be shown */
    Boolean	privatev;		/* if deps on private variables are to be shown */
    int         num_edges;	    /* current size of edgelist - not all slots
				       are necessary in use - what is this ?? (kats) */
    Query	query;		    /* query that describes this list
    				       of dependences: used in the dependence
				       filter mechanism. */
};



#endif
