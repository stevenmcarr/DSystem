/* $Id: map.h,v 1.4 1997/03/11 14:29:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef map_h
#define map_h

/* $Header $ */

#ifndef rn_varargs_h
#include <include/rn_varargs.h>
#endif


#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

/* the nesting_level argument is unused and present for compatibility only  */
EXTERN(int, mapStmt, (FortTreeNode stmt, int nesting_level, 
			      va_list arg_list));
#endif /* map_h */
