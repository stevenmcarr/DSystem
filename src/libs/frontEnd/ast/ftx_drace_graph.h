/* $Id: ftx_drace_graph.h,v 1.3 1997/03/11 14:29:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ftx_drace_graph_h
#define ftx_drace_graph_h


/*
 * 			      ftx_drace_graph.h
 * 									
 *	      part of ParaScope's AST transformation utility specialized
 *            to perform preprocessing required for datarace instrumentation
 *
 *   FUNCTION:
 *     augment the ftx_state object to with transformation functions
 *     relevant to instrumentation for detection of data races. This
 *     code presumes that the graph has been allocated and the standard
 *     transformations are represented in the graph
 *
 *   TO CUSTOMIZE:
 *	new transformation functions can be added; they should be
 *	declared in the list below
 *
 *   AUTHOR:
 *     John Mellor-Crummey
 *
 */

#include <libs/frontEnd/ast/ftxform.h>

/*
 * declarations of transformation functions
 */

TRANSFORMATION_FUNCTION(ftx_datarace);

TRANSFORMATION_FUNCTION(ftx_complete_datarace_transformations);

FTX_INIT_FUNCTION(ftx_init_datarace_graph);
FTX_ENABLE_FUNCTION(ftx_enable_datarace_graph);

#endif
