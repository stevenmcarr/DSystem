/* $Id: ftExportSimple.C,v 1.1 1997/03/11 14:29:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */

//************************************************************************
//
//                                ftExportSimple.C
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
//************************************************************************


#include <libs/frontEnd/ast/ftExportSimple.h>
#include <libs/frontEnd/prettyPrinter/sigcomments.h>


void ftExportSimple(FortTree ft, FortTextTree ftt, FILE *outf)
{
  SignificantCommentHandlers *schandlers = DEFAULT_SIG_COMMENT_HANDLERS;

  ftt_TreeChanged(ftt, ft_Root(ft)); // recompute text for tree
  ftt_ExportToFile(ftt, outf, '*', schandlers); // and export it
}
