/* $Id: analyse.h,v 1.9 1997/03/11 14:28:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * Header file for analyse.C
 */

/**********************************************************************
 * Revision History:
 * $Log: analyse.h,v $
 * Revision 1.9  1997/03/11 14:28:37  carr
 * newly checked in as revision 1.9
 *
 * Revision 1.9  94/03/21  13:05:41  patton
 * fixed comment problem
 * 
 * Revision 1.8  94/02/27  20:15:29  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.9  1994/02/27  19:47:03  reinhard
 * Added if2if_endif().
 *
 * Revision 1.8  1994/01/18  19:52:36  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.7  1993/06/15  16:11:10  reinhard
 * Deleted Action_type.
 *
 * Revision 1.6  1993/06/09  23:50:55  reinhard
 * Added loop analysis functions.
 *
 */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif
#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/*-------------------- CONSTANTS ------------------------------------*/

// Max string length of a reference
static const int MAX_REF_LENGTH = 100;   

// Return value for unknown # of iterations of a given loop
static const int UNKNOWN_SIZE = -1;      

/*------------------- TYPES ---------------------------------*/

// Type of a comparison function, as used by qsort or qcount.
//typedef int (*Compare_type)(const void *first, const void *second);
typedef FUNCTION_POINTER (int, Compare_type, (const void *first,
					      const void *second));

// Type of a function to add a node to an AST list.
typedef AST_INDEX (*ListInsertFunc)(AST_INDEX node, AST_INDEX element);
