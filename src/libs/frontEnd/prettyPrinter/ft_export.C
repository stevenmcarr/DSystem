/* $Id: ft_export.C,v 1.3 1997/03/11 14:30:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id */

//************************************************************************
//
//                                ft_export.C
//
//
//   INVOCATION:
//      retcode = ft_export ( ft, ftt, outf, mapf, dialect);
//
//   FUNCTION:
//      pretty print a FortTree into outf
//
//   RETURNS:
//      If everything is successful, a  0 is returned.
//      If there is a problem  ...    (it will exit or crash -- JMC)
//
//************************************************************************

#include <stdio.h>

#include <libs/support/misc/general.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/frontEnd/ast/ftxannot.h>

#include <libs/frontEnd/ast/ftx_stdgraph.h>
#include <libs/frontEnd/prettyPrinter/options_i.h>
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/frontEnd/prettyPrinter/sigcomments.h>


int ft_export(Context context, FortTree ft, FortTextTree ftt, FILE *outf, 
	      Company dialect) 
{
  SignificantCommentHandlers *schandlers = DEFAULT_SIG_COMMENT_HANDLERS;
  ftx_state s = ftx_create(ftx_init_std_graph);
  ftx_enable_std_graph_defaults(s);

  ftx_opt_dialect(s, dialect); // select the language dialect 

  // initialize annotations that correspond to the input arguments 
  ftx_put_annotation(s, FTX_FT, ft);
  ftx_put_annotation(s, FTX_FTT, (void *) ftt);
  ftx_put_annotation(s, FTX_MODULE_CONTEXT, (void *) context);
  ftx_put_annotation(s, FTX_OUTFILE_STREAM, outf);

  ft_AstSelect(ft);
  ftx_transform(s);

  // update the tree
  ftt_TreeChanged(ftt, ft_Root(ft));

  // and export it

  ftt_ExportToFile(ftt, outf, '*', schandlers);

  ftx_destroy(s);

  return 0;
}
