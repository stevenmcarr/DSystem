/* $Id: ftExportSimple.h,v 1.1 1997/03/11 14:29:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */

#ifndef ftExportSimple_h
#define ftExportSimple_h

/************************************************************************
//
//                                ftExportSimple.h
//
//
//   INVOCATION:
//      ftExportSimple ( ft, ftt, outf );
//
//   FUNCTION:
//      pretty print a FortTree into outf
//
//   RETURNS:
//      nothing. If there is a problem ... it will exit or crash.
//
/ ***********************************************************************/

#include <stdio.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

#ifndef FortTextTree_h
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#endif

EXTERN(void, ftExportSimple, (FortTree ft, FortTextTree ftt, FILE *outf));

#endif

