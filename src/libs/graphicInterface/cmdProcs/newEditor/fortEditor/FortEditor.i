/* $Id: FortEditor.i,v 1.4 1997/03/11 14:30:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/FortEditor.i					*/
/*									*/
/*	FortEditor -- abstract text/structure editor for Fortran	*/
/*	Last edited: July 3, 1990 at 4:10 pm				*/
/*									*/
/************************************************************************/





#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortVFilter.h>

#include <libs/graphicInterface/cmdProcs/newEditor/FortEditor.h>




/************************/
/*  Selection		*/
/************************/


typedef struct
  {
    /* text */
      int		line;
      int		sel1;
      int		sel2;

    /* structure */
      FortTreeNode	node;
      Boolean		structure;

  } Selection;




/*********************************/
/*  Operations for subparts only */
/*********************************/


EXTERN(void, ed__GetStatus,(FortEditor ed, Selection *sel, int *curLine,
                            TextString *curText, Boolean *dirty));
EXTERN(void, ed__SetStatus,(FortEditor ed, Selection sel, int curLine,
                            TextString curText, Boolean dirty));
EXTERN(void, ed__MakeClean,(FortEditor ed, Boolean put, Boolean struc, 
                            Boolean update, Selection *sel));





