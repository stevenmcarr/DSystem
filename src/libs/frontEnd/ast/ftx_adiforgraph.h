/* $Id: ftx_adiforgraph.h,v 1.3 1997/03/11 14:29:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ftxadiforgraph_h
#define ftxadiforgraph_h


/*
 * 			      ftxadifor.h
 * 									
 *	      part of ParaScope's AST transformation utility specialized
 *            to perform preprocessing required for ADIFOR
 *
 *   FUNCTION:
 *     augment the ftx_state object to with transformation functions
 *     relevant to preprocessing for automatatic differentiation. This
 *     code presumes that the graph has been allocated and the standard
 *     transformations are represented in the graph
 *
 *   TO CUSTOMIZE:
 *	new transformation functions can be added; they should be
 *	declared in the list below
 *
 *   AUTHOR:
 *     Alan Carle
 *
 */

#include <libs/frontEnd/ast/ftxform.h>

/*
 * declarations of transformation functions
 */
TRANSFORMATION_FUNCTION(ftx_declareImplicits);
TRANSFORMATION_FUNCTION(ftx_cleanFuncs);
TRANSFORMATION_FUNCTION(ftx_funcsToSubrs);
TRANSFORMATION_FUNCTION(ftx_hoistExprArgs);
TRANSFORMATION_FUNCTION(ftx_genericsToIntrinsics);
TRANSFORMATION_FUNCTION(ftx_complete_adifor_transformations);

FTX_INIT_FUNCTION(ftx_init_adifor_graph);
FTX_ENABLE_FUNCTION(ftx_enable_adifor_graph);
#endif
