/* $Id: private_dg.h,v 1.10 1997/03/11 14:35:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef	private_dg_h
#define	private_dg_h

/*************************************************************************

		File:		private_dg.h
		Author:		Kathryn McKinley

	This contains the private declarations for an implementation
	of dependence edges on multiple lists of level vectors and 
	references.  The dependence graph meets the specifications
	described in 'Dependence Graph Abstraction in PARASCOPE' by 
	Jaspal Subhlok. 
	
	All the data structures and routines described here are for
	internal use only, and should only be accessed through the
	routines provided in list_dg.h

**************************************************************************/


#ifndef	general_h
#include <libs/support/misc/general.h>
#endif

#ifndef	xstack_h
#include <libs/support/stacks/xstack.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#endif

#include <libs/frontEnd/fortTextTree/FortTextTree.h>

#define MIN_EDGES 100
#define MIN_LIST_HEADS 100
#define NOT_SET -1


/*--------------------------------------------------------------*/
/* the following definitions are needed for DG_Instance		*/

/* Doubly linked lists for edges that are on multiple lists. */ 
typedef struct dg_edge_list {
	EDGE_INDEX 	 next;
	EDGE_INDEX	 prev;
} DG_Edge_List;

/* headers for level vectors and references */
typedef struct dg_single {
	EDGE_INDEX	next;
} DG_Single;

typedef struct dg_single_list {
	DG_Single  src;
	DG_Single  sink;
	Boolean    used;
}DG_Single_List;

/* A header for a reference list indicating the source and sink of
 * a dependence.
 */
typedef struct dg_list {
	DG_Edge_List  src;
	DG_Edge_List  sink;
	Boolean	      used;	/* An internal flag indicating if this
				 * header is in use by a client. */
} DG_List;


/* A header for a level vector indicating the source and sink of
 * a dependence, the size of the vector, and the level in the 
 * vector.
 */
typedef struct dg_vector {
	DG_Single     src;
	DG_Single     sink;
	int	      size;
	Boolean	      used;
} DG_Vector;

/* A two link list of the free vectors */
typedef struct dg_free_vector {
	int		 index; /* its index in the DG_Vector array */
	int		 next_size;	/* the next bigger sized free chunk */
	int		 next_free;	/* the next same sized free chunk */
} DG_Free_VMD;

/*--------------------------------------------------------------*/
/* DG_Instance -- the dependence graph structure 		*/

struct struct_DG_Instance {

	/* edge structures */
	DG_Edge        *edgeptr;
	DG_List        *ref_list;  /* These match the edges exactly */
	DG_List        *vec_list;  /* and link interdependent edges */
	
	Stack           fstack;	   /* stacks the indexes of free edges */
	int	        num_edges; /* total allocated */
	int	        num_free;  /* total number not in use */
	
	/* reference structures */
	DG_Single_List	*ref;
	Stack		 ref_fstack;
	int		 num_ref;
	int		 num_free_ref;
	
	/* the array of variable sized vectors, these are used similarly to
	 * memory allocation in multiprocessors.  VMD stands for vector
	 * memory discriptor.
	 */
	DG_Vector	*vmd;
	DG_Free_VMD     *free_vmd;
	Stack		 free_vmd_stack;
	int		 num_vmd;
	int		 num_free_vmd;
	int		 first_free_vmd;

	/* Characteristics of this dependence graph		*/
    
	GraphType external_analysis;	/* externally generated dep info */
	char	*dependence_header;	/* header with options from graph file	*/
	Boolean	 set_interchange;	/* update interchange info during testing */
	Boolean	 local_analysis;	/* analyze locally			*/
	Boolean	 input_dependences;	/* compute input dependences		*/

} ;


#define	DG_EXTERNAL_ANALYSIS(dg)	(dg->external_analysis)
#define	DG_DEPENDENCE_HEADER(dg)	(dg->dependence_header)
#define	DG_LOCAL_ANALYSIS(dg)		(dg->local_analysis)
#define	DG_INPUT_DEPENDENCES(dg)	(dg->input_dependences)
#define	DG_SET_INTERCHANGE(dg)		(dg->set_interchange)



/* Internal routines called outside of their files. */
/* in vector_dg.c	*/
EXTERN(void, dg_create_level_vectors,
		( DG_Instance * dg, int  num_vectors ) );

/* in ref_dg.c		*/
EXTERN(void, dg_create_ref_lists,
		( DG_Instance * dg, int  num_refs ) );

EXTERN(int, dg_alloc_ref_list, (DG_Instance *dg));

EXTERN(void, dg_free_ref_list, (DG_Instance *dg, int ref));

EXTERN(void, set_all_ref_free, (DG_Instance *dg, int start, int num_refs));


EXTERN(Boolean, dg_save_edge, (FortTextTree ftt, DG_Edge *edge, FILE *depFP));

#endif
