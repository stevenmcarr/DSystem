/* $Id: el_header.h,v 1.4 1997/03/11 14:35:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/***************************************************************************

  Prototypes for el functions.

 ****************************************************************************/

#ifndef el_header_h
#define el_header_h


#ifndef	general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	li_instance_h
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	cd_graph_h
#include <libs/moduleAnalysis/dependence/controlDependence/cd_graph.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#endif

/**********************************************
  routines to support the EL_Instance structure		from /el/el.c
 **********************************************/

/* -----------------------------------------------------------------------
   el_create_instance() - create an edge list structure.
 ----------------------------------------------------------------------- */
EXTERN(  EL_Instance *,	 el_create_instance, ( int num_edges ) );

EXTERN(  void,    	  el_destroy_instance, ( EL_Instance * EL ) );

/*	"true" if loop carried deps are to be shown 
 */
EXTERN(  Boolean,	  el_show_loopCarried, ( EL_Instance * el ) );

/*	returns the number of loop carried dependences on shared variables
 */
EXTERN(  int,		  el_num_lc_shared, ( EL_Instance * el ) );


/* -----------------------------------------------------------------------
   el_copy_active_edge_list_structure() - Create a copy of an edge list 
   structure
   ----------------------------------------------------------------------- */
EXTERN(  EL_Instance *,	 el_copy_active_edge_list_structure, 
		( EL_Instance	* oldel ) );


/*---------------------------------------------------------------------------
  el_copy_edge() - copies DG edge
 -------------------------------------------------------------------------- */
EXTERN(  void, 		 el_copy_edge,
		( DG_Instance	* dg, DT_info	* dt, DG_Edge	* edgeptr,
		 EDGE_INDEX	 oldedge, EDGE_INDEX	 newedge ) );




/**********************************************
  routines to support the dependence display.  from /el/el.c
 **********************************************/

/* -----------------------------------------------------------------------
   el_new_loop() - collect the list of dependences for this loop in an edge
   list structure, and return the number of dependences found.
   "node" is the ast index of the loop header statement. 
  -----------------------------------------------------------------------*/
EXTERN(  int,     	el_new_loop, ( EL_Instance * el, LI_Instance * li,
		SideInfo * infoPtr, DG_Instance * dg, AST_INDEX  loop) );

EXTERN(  int,   el_total_num_deps, ( EL_Instance * el ));

EXTERN(  Boolean, 	edge_is_active, ( EL_Instance	* el, int i ) );

/*----------------------------------------------------------------------
   get_dependence() - get the index of this edge in the dep edge list if
   		 	the dep edge is active.
  ----------------------------------------------------------------------*/
EXTERN(  EDGE_INDEX,  	get_dependence, ( EL_Instance	* el, int i ) );

/*----------------------------------------------------------------------
   first_dependence() - get the first dependence edge, and return its
   edge_index.
  ----------------------------------------------------------------------*/
EXTERN(  EDGE_INDEX,  	first_dependence, ( EL_Instance	* el ) );

/*----------------------------------------------------------------------
   next_dependence() - get the next dependence edge, and return its
   edge_index. 
  ----------------------------------------------------------------------*/
EXTERN(  EDGE_INDEX,  	next_dependence, ( EL_Instance	* el ) );

/*-----------------------------------------------------------------------
   prev_dependence() - get the prev dependence edge, and return its
   edge_index. 
  ----------------------------------------------------------------------*/
EXTERN(  EDGE_INDEX,  	prev_dependence, ( EL_Instance	* el ) );



/*******************************************************
  routines to support the dependence filter mechanism.	from /el/el.c
 *******************************************************/
EXTERN(	void,		el_hide, 
		( EL_Instance * el, DG_Instance	* dg, 
		 char * list1, char * list2,
		 AST_INDEX ind1, AST_INDEX ind2, 
		 char * list3, char * list4) );
EXTERN(	void,		el_show, 
		( EL_Instance * el, DG_Instance	* dg, 
		 char * list1, char * list2,
		 AST_INDEX ind1, AST_INDEX ind2, 
		 char * list3, char * list4) );
EXTERN(  void,		el_remove, 
		( EL_Instance * el, DG_Instance	* dg, 
		 char * list1, char * list2,
		 AST_INDEX ind1, AST_INDEX ind2, 
		 char * list3, char * list4) );
EXTERN(	void,		el_showall, 
		( EL_Instance * el ) );
EXTERN(	void,		el_sort, 
		( EL_Instance * el, DG_Instance	* dg, char * sortlist ) );

EXTERN(	void,		el_query_convert,
		( EL_Instance	*el ) );
/*	   This routine converts the dependence type information into
 *	a text format.
 */

EXTERN(	void,		el_query_init,
		( EL_Instance	*el, char	*type, char	*text_name,
		 AST_INDEX	 src_ast, AST_INDEX	 sink_ast,
		 char		*text_dims, char	*text_block) );
/*	This routine initializes the fields in the el->query structure.
 */ 


EXTERN( void,         el_query_free,
              ( EL_Instance *el));


/*************************************************************************
   routines to support statement insertion/deletion and variable renaming. 
 *************************************************************************/

/*
 * el_transfer_edges(): transfer all dependence edges into and out of
 * node1 to node2.
 * 	AST_INDEX	node1;
 *	AST_INDEX	node2;
 */
/*	this function is not defined or used anywhere in cfg/ or ped_cp/ */ 
/* 	EXTERN( void, el_transfer_edges, ( ) );		*/


/**********************************************************************
 routines to support the code generator.		from /el/el.c
 **********************************************************************/

/*
 * el_get_dims (): get the dimensions of edge_index
 *  Generic  elg;
 *  int	    edge_index;
 */
EXTERN( int,	el_get_dims, ( EL_Instance * el, int num ) );

/*
 * el_get_block(): get the common block of edge_index
 *  Generic  elg;
 *  int	     edge_index;
 */
EXTERN( char *, el_get_block, ( EL_Instance * el, int num ) );

/*
 * el_gen_get_text(): returns the character string of index
 */
EXTERN( char *, el_gen_get_text, ( AST_INDEX index ) );


/**************************************************** /el/view.c ******
 * Activates and sets a permanent dependence filter with the options:
 * 	-All edges (loop carried, loop independent, control)
 *	-loop carried and control edges only
 *	-loop carried edges only
 **********************************************************************/


/*	Sets view to include loop carried dependences.			*/
EXTERN( void, el_set_view_lc, ( EL_Instance * el, Boolean lc ) );

/*	Sets view to include control dependences.			*/
EXTERN( void, el_set_view_control, ( EL_Instance * el, Boolean control ) );

/*	Sets view to include loop independent dependences.		*/
EXTERN( void, el_set_view_li, ( EL_Instance * el, Boolean li ) );

/*	Sets view to include dependences on private variables.		*/
EXTERN( void, el_set_view_private, ( EL_Instance * el, Boolean privatev ) );


/*	Returns if the loop carried dependences are being viewed.	*/
EXTERN( Boolean, el_get_view_lc, ( EL_Instance * el ) );

/*	Returns if the control dependences are being viewed.		*/
EXTERN( Boolean, el_get_view_control, ( EL_Instance * el ) );

/*	Returns if the loop independent dependences are being viewed.	*/
EXTERN( Boolean, el_get_view_li, ( EL_Instance * el ) );

/*	Returns if the dependences on private variables are being viewed.	*/
EXTERN( Boolean, el_get_view_private, ( EL_Instance * el ) );


/*	**********	The Following are Never Used or Defined	*********/

/*
 * el_get_..() and el_put_.. () - routines to reference and allocate the
 * level vectors for each statement and the ref lists for each variable in
 * the Rn tree.
 */

/* EXTERN( int, el_get_level_vector, ( ) ); */
/*     AST_INDEX       a; */

EXTERN( void, el_put_level_vector, ( ) );
/*     AST_INDEX       a;
 *     int             b;
 */

EXTERN( int, el_get_ref_ptr, ( ) );
/*    AST_INDEX       a; */

EXTERN( void, el_put_ref_ptr, ( ) );
/*    AST_INDEX       a;
 *    int             b;
 */

/*	Finds the statement ast for src, and sink, storing it in
 *	Edge_array.  Also finds the subscripts, if any, for src
 *	and sink, storing it in Edge_array.  If src or sink is
 *	<= AST_NIL, AST_NIL is stored.
 */
/*	Never Used or Defined.						*/
EXTERN( void,	el_put_dep_ast_info, 
		( DG_Edge * Edge_array, int e, AST_INDEX src, AST_INDEX sink) );



#endif	/* el_instance_h */
