/* $Id: dg_header.h,v 1.7 1997/03/27 20:46:14 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


#ifndef dg_header_h
#define dg_header_h


/*************************************************************************

		file:	dg_header.h

	This contains the public declarations for the functions to
	access the dependence graph. All the routines and data 
	structures described here may be used or accessed by programs 
	wishing to create, change, or peruse a dependence graph.

**************************************************************************/


#ifndef	general_h
#include <libs/support/misc/general.h>
#endif
#ifndef	ast_h
#include <libs/frontEnd/ast/ast.h>
#endif
#ifndef	newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif
#ifndef	FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif
#ifndef	FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif
#ifndef	side_info_h
#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#endif
#ifndef	dt_info_h
#include <libs/moduleAnalysis/dependence/dependenceTest/dt_info.h>
#endif
#ifndef	depType_h
#include <libs/moduleAnalysis/dependence/interface/depType.h>
#endif
#ifndef	dg_instance_h
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#endif
#ifndef	li_instance_h
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#endif
#ifndef	el_instance_h
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#endif
#ifndef cfg_h
#include <libs/moduleAnalysis/cfg/cfg.h>
#endif


           /************************************************/
           /* To initialize dependence graph structures.   */
           /************************************************/

EXTERN( void, dg_all,
		(Context     module_context,
		 Context     mod_in_prog_context, 
		 Context     prog_context,
		 FortTextTree ftt, FortTree  ft, DG_Instance **DG,
		 EL_Instance **EL, LI_Instance **LI, SideInfo  **SI,
		 DT_info **DT,CfgInfo *cfgModule, 
		 Boolean InputDep) );

EXTERN(Boolean, dg_save_instance, (FortTextTree ftt, DG_Instance *dg_instance, 
				   FILE *depFP));


	/************************************************/
	/* To create a dependence graph instance.	*/
	/************************************************/

EXTERN(DG_Instance *,  dg_create_instance, (void));
/* (no parameters)
 * 	This creates a pointer to an instance structure for a single
 * 	dependence graph.  This is passed as DG to all other routines.
 */

	/************************************************/
	/* To destroy a dependence graph instance.	*/
	/************************************************/

EXTERN(void, dg_destroy, (DG_Instance * dg));
/*
 *	This frees all internal structures,
 *	then the memory for the DG_Instance itself.
 */



	/* ============================================================ */
	/* Functions to work with Dependence Related Files in Database	*/
	/* ============================================================ */



EXTERN( void,	dg_graph_filename,
		( Context	 module_context, char	*filename ));

EXTERN( void,	dg_index_filename, 
		( Context	 module_context, char	*filename ));

EXTERN( void,	dg_rsd_filename, 
		( Context	 module_context, char	*filename ));

/************************************************/
/* Read a dependence graph from a file		*/
/* Return false if fails and new DG_Instance,	*/
/* Else   true, DG_Instance and LI_Instance	*/
/************************************************/
EXTERN( Boolean, dg_readgraph, 
		( DG_Instance ** DG, LI_Instance ** LI, 
		 Context module_context,
		 Context mod_in_prog_context,
		 Context prog_context,
		 FortTextTree ftt, FortTree ft, SideInfo * infoPtr,
		 DT_info * dt_info, AST_INDEX root, CfgInfo cfgModule) );

/********************************************************/
/* Open and Close files for dependence structures	*/
/********************************************************/
EXTERN( Boolean,	dg_open_files,
       	        ( Context module_context,
		 Context  mod_in_prog_context,
		 Context  prog_context,
		 char   * access,
		 FILE **gptr, FILE **iptr, FILE **rsdptr));

EXTERN( void,	dg_close_files,
		(FILE *gptr, FILE *iptr, FILE *rsdptr));

/*****************************************************************************/
/* Verify that the AST was not modified after the Dependence Graph was saved */
/*****************************************************************************/
EXTERN( Boolean,	dg_check_file_date, 
		( Context  module_context1,
		 Context  mod_in_prog_context,
		 Context  module_context2 ));

/********************************************************/
/* Return the type of dependence graph stored on disk	*/
/********************************************************/
EXTERN( GraphType,	dg_get_graph_type,
		( char	 depFile_path[DB_PATH_LENGTH] ));



	/* ============================================================ */
	/* Routines for manipulation of dependence data structures	*/
	/* ============================================================ */



/********************************************/
/* To create and update edges on all lists. */
/********************************************/
EXTERN(DG_Edge *,  dg_create_edge_structure, 
		(DG_Instance * DG, int num_edges ));
/* This returns a pointer to the edge array for the DG.  Num_edges
 * is the number of edges which should initially be allocated.
 * All edges are marked unused at this time.  This is passed as
 * edgeptr to many other routines.
 */

EXTERN(DG_Edge *,  dg_get_edge_structure, (DG_Instance * DG));
/* This returns a pointer to the edge array for the DG.  
 */

EXTERN(EDGE_INDEX, dg_alloc_edge, (DG_Instance * DG, DG_Edge **edgeptr));
/* Returns an index into the edge array, edgeptr, of an edge not 
 * currently in use.  The edge is marked as used, and all its other 
 * fields are initialized to be an unset value.
 */

EXTERN(void, dg_free_edge, 
		(DG_Instance * DG, DG_Edge *edgeptr, EDGE_INDEX edge));
/* Puts edge, a member of edgeptr, on the free stack, and marks it 
 * unused.
 */

EXTERN(void, dg_add_edge, (DG_Instance * DG, EDGE_INDEX edge));
/* Takes the edge and inserts it into all the appropriate lists.
 * The following fields in the edge must be filled in and are 
 * created by the dg_alloc_ calls for level vectors and
 * reference lists:
 *		src_vector;	handle to src level_vector
 *		sink_vector;	handle to sink level_vector
 *		src_ref;	handle to src reference list
 *		sink_ref;	handle to sink reference list
 */

EXTERN(void, dg_delete_edge, (DG_Instance * DG, EDGE_INDEX edge));
/* Deletes the edge off all lists, but does not put on free stack.
 * Edge can be modified and reinserted using dg_add_edge()
 */

EXTERN(void, dg_delete_free_edge, (DG_Instance * DG, EDGE_INDEX edge));
/* Calls dg_delete_edge() followed by dg_free_edge().
 */


/****************************************************************/
/* To create and update level vectors associated with statement */
/* dependencies. 						*/
/****************************************************************/
EXTERN(int, dg_alloc_level_vector, (DG_Instance * DG, int size));
/* Returns a handle for a new level vector with size number of 
 * possible entries.
 */

EXTERN(void, dg_free_level_vector, (DG_Instance * DG, int vector));
/* Put vector on the reference free list.
 */

/*****************************************************************/
/* To create and update the reference lists associated with each */
/* reference involved in a dependence.				 */
/*****************************************************************/
EXTERN(int, dg_alloc_ref_list, (DG_Instance * DG));
/* Returns a handle for a new reference list.
 */

EXTERN(void, dg_free_ref_list, (DG_Instance * DG, int	ref));
/* Put ref on the reference free list.
 */

/***********************************************************/
/*  To walk the level vectors and the expression pointers. */
/***********************************************************/

EXTERN(EDGE_INDEX, dg_first_src_stmt, 
		(DG_Instance * DG, int src_vec, int level));
/* Returns the first dependence edge whose src statement has the 
 * level vector, src_vec, where the dependence is carried at level 
 * level.
 */

EXTERN(EDGE_INDEX, dg_first_sink_stmt, 
		(DG_Instance * DG, int sink_vec, int level));
/* Returns the first dependence edge whose sink statement has the 
 * level vector, sink_vec, where the dependence is carried at level 
 * level.
 */

EXTERN(EDGE_INDEX, dg_first_src_ref, (DG_Instance * DG, int src_ref));
/* Returns the first dependence edge whose src reference is sink_vec.
 */

EXTERN(EDGE_INDEX, dg_first_sink_ref, (DG_Instance * DG, int sink_ref));
/* Returns the first dependence edge whose sink reference is sink_vec.
 */

EXTERN(EDGE_INDEX, dg_next_src_stmt, (DG_Instance * DG, int edge));
/* Returns the next dependence edge which has the same source statement
 * as edge, where edge may be obtained from dg_first_src_stmt.
 */

EXTERN(EDGE_INDEX, dg_next_src_ref, (DG_Instance * DG, int edge));
/* Returns the next dependence edge which has the same source reference
 * as edge, where edge may be obtained from dg_first_src_ref.
 */

EXTERN(EDGE_INDEX, dg_next_sink_stmt, (DG_Instance * DG, int edge));
/* Returns the next dependence edge which has the same sink statement
 * as edge, where edge may be obtained from dg_first_sink_stmt.
 */

EXTERN(EDGE_INDEX, dg_next_sink_ref, (DG_Instance * DG, int edge));
/* Returns the next dependence edge which has the same sink reference
 * as edge, where edge may be obtained from dg_first_sink_stmt.
 */

EXTERN(EDGE_INDEX, dg_prev_src_stmt, (DG_Instance * DG, int edge));
/* Returns the prev dependence edge which has the same source statement
 * as edge.
 */
EXTERN(EDGE_INDEX, dg_prev_src_ref, (DG_Instance * DG, int edge));
/* Returns the prev dependence edge which has the same source reference
 * as edge.
 */

EXTERN(EDGE_INDEX, dg_prev_sink_stmt, (DG_Instance * DG, int edge));
/* Returns the prev dependence edge which has the same sink statement
 * as edge.
 */
EXTERN(EDGE_INDEX, dg_prev_sink_ref, (DG_Instance * DG, int edge));
/* Returns the next dependence edge which has the same sink reference
 * as edge.
 */

EXTERN(int, dg_length_level_vector, (DG_Instance * DG, int vec));
/* Returns the number of entries in the level vector, vec,
 * (i.e. the maximum loop nesting depth).
 */



	/* ==================================================== */
	/* Functions to access fields within DG_Instance	*/
	/* ==================================================== */


/*----------------------------------------- external_analysis	*/

EXTERN( void,	dg_set_external_analysis, 
		( DG_Instance	*dg, GraphType	 gtype ));

EXTERN( GraphType,	dg_get_external_analysis, ( DG_Instance	*dg ));

/*----------------------------------------- set_interchange	*/

EXTERN( void,	dg_set_set_interchange, 
		( DG_Instance	*dg, Boolean	 flag ));

EXTERN( Boolean,	dg_get_set_interchange, ( DG_Instance	*dg ));

/*----------------------------------------- dependence_header	*/

EXTERN( void,	dg_set_dependence_header, 
		( DG_Instance	*dg, char *	 str ));

EXTERN( char *, 	dg_get_dependence_header, ( DG_Instance	*dg ));

/*----------------------------------------- local_analysis	*/

EXTERN( void,	dg_set_local_analysis, 
		( DG_Instance	*dg, Boolean	 flag ));

EXTERN( Boolean,	dg_get_local_analysis, ( DG_Instance	*dg ));

/*----------------------------------------- input_dependences	*/

EXTERN( void,	dg_set_input_dependences, 
		( DG_Instance	*dg, Boolean	 flag ));

EXTERN( Boolean,	dg_get_input_dependences, ( DG_Instance	*dg ));

/*********************************************************************/

EXTERN(Boolean, readgraph, (FILE *gfile_ptr, MapInfoOpaque *map, DG_Instance *DG,
                            DT_info *dt, SideInfo *infoPtr, FortTextTree ftt));

EXTERN(void, readrsd, (FILE *rsdptr, MapInfoOpaque *map, DG_Instance *dg,
                       DT_info *dt, SideInfo *infoPtr, FortTextTree ftt));

EXTERN(DepType, get_dtype, (char dchar));

EXTERN(Boolean, pfc_bogus, (char *name));

EXTERN(void, read_free, (char *str1, char *str2, char *str3, char *str4));

EXTERN(char*, dg_var_name, (DG_Edge *Edge));

#endif	/* dg_instance_h */












