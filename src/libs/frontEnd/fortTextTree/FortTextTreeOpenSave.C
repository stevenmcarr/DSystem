/* $Id: FortTextTreeOpenSave.C,v 1.1 1997/03/11 14:29:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortTextTreeOpenSave.ansi.c			*/
/*									*/
/*	Open and Save operations for a FortTextTree 			*/
/*									*/
/************************************************************************/

#include <ctype.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/arrays/FlexibleArray.h>
#include <include/bstring.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>
#include <libs/frontEnd/fortTextTree/FortUnparse1.h>
#include <libs/frontEnd/fortTextTree/FortUnparse2.h>
#include <libs/frontEnd/fortTextTree/FortParse1.h>
#include <libs/frontEnd/fortTextTree/FortParse2.h>

#include <libs/support/memMgmt/mem.h>
#if 0
#include <libs/frontEnd/prettyPrinter/sigcomments.h>

#include <include/rn_varargs.h>
#include <unistd.h>
#include <string.h>

#include <libs/frontEnd/fortTextTree/MapInfo_c.h>
#endif



/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/


/************************/
/*  Representation	*/
/************************/

/* File handle for error reporting */ 
static DB_FP *errors_fp;





/************************/
/*  Initialization info	*/
/************************/

/* name of the (new) saved-state attribute */
char * ftt_saveAttribute = "FortTextTree";


/************************/
/* Forward declarations	*/
/************************/


static void banner(char *banner_msg);
static void badReporter(int lineno, char *linetext, char *errortext);
static void ftt_import(Context context, FortTextTree ftt);


/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/


FortTextTree ftt_Open(Context context, DB_FP *, FortTree ft)
{
  FortTextTree ftt;
  TextTree tt;
  static Generic zeros[tt_EXTRASIZE];
  DB_FP * this_fp;
  int dummy;

  /* allocate a new instance */
    ftt = (FortTextTree) get_mem(sizeof(ftt_Repr),"FortEditor:FortTextTree");
    if( (Generic) ftt == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(ftt)->ft = ft;

    /* determine where to get saved state */
      ft_States needs_reparsing = ft_GetState(ft);

      if( (context != CONTEXT_NULL) && (ft_GetState(ft) == ft_UNINITIALIZED) )
        this_fp = annotOpen(context, ftt_saveAttribute, "r");
      else
        this_fp = DB_NULLFP;

      /* check for nonexistent attribute */
      if( db_buffered_read(this_fp, (char*)&dummy, sizeof(int)) == 0 )
      {
          annotClose(this_fp);
          this_fp = DB_NULLFP;
      }

    /* initialize side array needed by TextTree */
      bzero((char *) zeros, sizeof(zeros));
      R(ftt)->sideArray = ft_AttachSideArray(ft, tt_EXTRASIZE, zeros);
      if( this_fp != DB_NULLFP )
        ft_ReadSideArray(ft, R(ftt)->sideArray, context, this_fp);

    /* open the TextTree */
      tt = tt_Open(context, this_fp, (TT_Tree) ftt, (TT_TreeNode) ft_Root(ft),
                   &ftt_methods, sizeof(fx_StatToken));
      R(ftt)->tt = tt;

    /* create an empty expansion list */
      R(ftt)->expansions = flex_create(sizeof(fx_Expansion));

    /* close the saved state if any */
      if( this_fp != DB_NULLFP )  db_buffered_close(this_fp);

    /* if necessary, fill it in by parsing ... */
      if (needs_reparsing)
      {
        ftt_ImportFromFile(context, ft, &ftt);

	if (ft_Root(ft) != AST_NIL)
	{
	  /* don't re-save the reference source, we just parsed it! */

	  /* write out annotations for this module ... */
             ftt_Save(ftt, context, DB_NULLFP);

	  /* Write out the FortTree */
             ft_Save(R(ftt)->ft, context, DB_NULLFP);
	}
      }

  return ftt;
}


void ftt_Save(FortTextTree ftt, Context context, DB_FP *)
{
  DB_FP * this_fp;
  int dummy = 0;
  Boolean has_errors;

  /* select the tree */
  ft_AstSelect(R(ftt)->ft);

  /* dump out the errors annotation */
  errors_fp = annotOpen(context, "Module_Errors", "w");
  ftt_ReportErrors(context, ftt, banner, NULL, badReporter, &has_errors);
  annotClose(errors_fp);

  this_fp = annotOpen(context, ftt_saveAttribute, "w");

  /* write a dummy int for nonemptiness check at open time */
  (void) db_buffered_write(this_fp, (char*)&dummy, sizeof(int));
  ft_WriteSideArray(R(ftt)->ft, R(ftt)->sideArray, context, this_fp);

  tt_Save(R(ftt)->tt,context, this_fp);
  annotClose(this_fp);
}


/*
 * Author: Alan Carle, June 25, 1992
 */

void ftt_ImportFromFile(Context context, FortTree, FortTextTree *ftt_p)
{
  ftt_import(context, *ftt_p);
}


/* ftt_ReportErrors(ftt) -- rudimentary printing of errors in ftt */ 
void ftt_ReportErrors(Context context, FortTextTree ftt, ftt_banner_callback banner, 
                      ftt_goodline_callback good, ftt_badline_callback bad, 
                      Boolean *has_errors_p)
{
  char  *linetext;
  char  message[200];
  int   lineCount;

  /* ft_Check returns true if no errors */
  *has_errors_p = (Boolean) !ft_Check(R(ftt)->ft);

  ftt_TreeChanged(ftt, ft_Root(R(ftt)->ft));

  if (banner != NULL && *has_errors_p)
  {
    char *msg = nssave(3, "Errors encountered in module ", ctxLocation(context), ".");
    banner(msg);
    sfree(msg);
  }
  for (lineCount = 0; lineCount < ftt_NumLines(ftt); lineCount++)
  {
    if (ftt_IsErroneous(ftt, lineCount))
    {
      ftt_GetErrorMessage(ftt,lineCount,message);
      linetext = ftt_GetTextLine(ftt,lineCount);
      if (bad != NULL)
	bad(lineCount,linetext, message);
    }
    else
    {
      linetext = ftt_GetTextLine(ftt,lineCount);
      if (good != NULL)
	good(lineCount,linetext);
    }
  }
}




/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/


static void ftt_import(Context context, FortTextTree ftt)
{
  char *loc;
  loc = ctxLocation(context);
   ftt_ImportFromTextFile(loc, ftt);
}


/*
 * Specific instantiations of auxiliary functions for error reporting.
 * The file pointer "errors_fp" should be passed into ReportErrors.
 */ 
static void banner(char *banner_msg)
{
  io_fprintf(errors_fp, "%s\n", banner_msg);
}

static void badReporter(int, char *linetext, char *errortext)
{
  io_fprintf(errors_fp, "%s\n", linetext);
  io_fprintf(errors_fp, "%s\n", errortext);
}
