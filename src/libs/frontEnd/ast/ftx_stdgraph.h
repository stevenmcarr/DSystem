/* $Id: ftx_stdgraph.h,v 1.3 1997/03/11 14:29:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ftx_stdgraph_h
#define ftx_stdgraph_h


/*
 * 			      ftXform_i_h
 * 									
 *	      part of ParaScope's AST transformation utility
 * 									
 *   FUNCTION:
 *	a specific instance of an initializer for an ftx_graph object
 *
 *	this one creates the standard ParaScope transformation graph
 *
 *   TO CUSTOMIZE:
 *	new transformation functions can be added; they should be
 *	declared in the list below
 *
 *	their dependences should be described in the file "ftXform_i.c"
 *
 *   AUTHOR:
 *	Robert Hood
 */

#include <libs/frontEnd/ast/ftxform.h>

/*
 * declarations of transformation functions
 */
TRANSFORMATION_FUNCTION(ftx_blockIF);
TRANSFORMATION_FUNCTION(ftx_noELSEIF);
TRANSFORMATION_FUNCTION(ftx_DOtarget);
TRANSFORMATION_FUNCTION(ftx_labelDO);
TRANSFORMATION_FUNCTION(ftx_cleanSubs);
TRANSFORMATION_FUNCTION(ftx_profile);
TRANSFORMATION_FUNCTION(ftx_sequent);
TRANSFORMATION_FUNCTION(ftx_expandStmtFuncs);
TRANSFORMATION_FUNCTION(ftx_cray);
TRANSFORMATION_FUNCTION(ftx_ibm);
TRANSFORMATION_FUNCTION(ftx_noIBMpf);
TRANSFORMATION_FUNCTION(ftx_open_dg);
TRANSFORMATION_FUNCTION(ftx_close_dg);
TRANSFORMATION_FUNCTION(ftx_open_cg);
TRANSFORMATION_FUNCTION(ftx_close_cg);

TRANSFORMATION_FUNCTION(ftx_create_mapData);
TRANSFORMATION_FUNCTION(ftx_destroy_mapData);
TRANSFORMATION_FUNCTION(ftx_setup_complete);
TRANSFORMATION_FUNCTION(ftx_transformations_complete);

FTX_INIT_FUNCTION(ftx_init_std_graph);
FTX_ENABLE_FUNCTION(ftx_enable_std_graph_defaults);
#endif
