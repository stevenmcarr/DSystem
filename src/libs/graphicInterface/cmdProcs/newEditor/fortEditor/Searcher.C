/* $Id: Searcher.C,v 1.1 1997/06/25 13:43:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/Searcher.c					*/
/*									*/
/*	Searcher -- text and tree pattern matcher for a FortEditor	*/
/*	Last edited: July 3, 1990 at 10:34 am				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/Searcher.h>

#include <libs/graphicInterface/oldMonitor/include/dialogs/find.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    /* creation parameters */
      FortEditor           editor;

    /* status */
      Boolean              textPattern;
      char *               textPatString;
      Boolean              textPatFold;

  } srch_Repr;

#define	R(ob)		((srch_Repr *) ob)




/*************************/
/* Global search limits  */
/*************************/


struct
  {
    int     line;
    int     col;
    Boolean wrap;
    Boolean wrapped;

  } srch_limit;




/*************************/
/*  Forward declarations */
/*************************/

typedef FUNCTION_POINTER(Boolean,matchStringFunc,(Searcher,TextString,int,int,
                             		          Boolean,Boolean,int *));



STATIC(void,	selectionToStart,(Searcher srch, int line1, int sel1, int sel2,
				  Boolean dir, Boolean zero, int *startLine, 
                                  int *startCol));
STATIC(Boolean,	findLimited,(Searcher srch, Boolean dir, matchStringFunc matcher));
STATIC(int,	firstOfLine,(FortEditor ed, Boolean dir, int line));
STATIC(int,	lastOfLine,(FortEditor ed, Boolean dir, int line));
STATIC(Boolean,	moreLines,(FortEditor ed, Boolean dir, int *line));
STATIC(Boolean,	moreColumns,(FortEditor ed, Boolean dir, int line, int col));
STATIC(Boolean,	searchLine,(Searcher srch, matchStringFunc matcher, Boolean dir, int line,
                            int *col, int *matchlen));
STATIC(Boolean,	matchString,(Searcher srch, TextString text, int pos, int last,
                             Boolean dir, Boolean fold, int *matchlen));
STATIC(Boolean,	matchPlaceholder,(Searcher srch, TextString text, int pos, int last,
                                  Boolean dir, Boolean fold, int *matchlen));
STATIC(void,	setLimit,(int line, int col, Boolean wrap));
STATIC(Boolean,	obeyLimit,(int line));





/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void srch_Init()
{
  /* nothing */
}




void srch_Fini()
{
  /* nothing */
}




/*ARGSUSED*/

Searcher srch_Open(Context context, DB_FP *fp, FortEditor ed)
{
  Searcher srch;

  /* allocate a new instance */
    srch = (Searcher) get_mem(sizeof(srch_Repr),"FortEditor:Searcher");
    if( (Generic) srch == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(srch)->editor = ed;

    /* set status */
      R(srch)->textPattern = true;
      R(srch)->textPatString = ssave("");
      R(srch)->textPatFold = false;

  return (Generic) srch;
}




void srch_Close(Searcher srch)
{
  sfree(R(srch)->textPatString);
  free_mem((void*) srch);
}




/*ARGSUSED*/

void srch_Save(Searcher srch, Context context, DB_FP *fp)
{
  /* ... */
}






/************************/
/*  Patterns		*/
/************************/




/*ARGSUSED*/

void srch_SetPattern(Searcher srch, FortVFilter filter)
{
  /* ... */

  R(srch)->textPattern = false;
}




void srch_SetPatternText(Searcher srch, char *str, Boolean fold)
{
  R(srch)->textPattern   = true;
  R(srch)->textPatString = ssave(str);
  R(srch)->textPatFold   = fold;
}






/************************/
/*  Search control	*/
/************************/




Boolean srch_Find(Searcher srch, Boolean dir, Boolean wrap)
{
  int line,sel1,sel2,limitLine,limitCol;

  ed_GetSelection(R(srch)->editor,&line,&sel1,&sel2);
  selectionToStart(srch,line,sel1,sel2,dir,false,&limitLine,&limitCol);
  setLimit(limitLine,limitCol,wrap);

  return findLimited(srch, dir, matchString);
}




Boolean srch_FindPlaceholder(Searcher srch, Boolean dir, Boolean wrap)
{
  int line,sel1,sel2,limitLine,limitCol;

  ed_GetSelection(R(srch)->editor,&line,&sel1,&sel2);
  selectionToStart(srch,line,sel1,sel2,dir,true,&limitLine,&limitCol);
  setLimit(limitLine,limitCol,wrap);

  return findLimited(srch, dir, matchPlaceholder);
}




/*ARGSUSED*/

int srch_ReplaceText(Searcher srch, Boolean dir, Boolean global, 
                     Boolean all, char *newStr)
{
  FortEditor ed = R(srch)->editor;
  int line,sel1,sel2,limitLine,limitCol,numReplaced;
  Boolean more;

  if( global )
    { ed_BeginEdit(ed);
      beginComputeBound();
      ed_GetSelection(ed,&line,&sel1,&sel2);
      selectionToStart(srch,line,sel1,sel2,dir,false,&limitLine,&limitCol);
      setLimit(limitLine,limitCol,all);

      numReplaced = 0;
      more = true;
      while( more )
        if( findLimited(srch,dir,matchString) )
          { (void) srch_ReplaceText(srch,dir,false,false,newStr);
            numReplaced += 1;
          }
        else
          more = false;

      ed_SetSelection(ed,line,sel1,sel2);
      endComputeBound();
      ed_EndEdit(ed);
    }
  else
    { ed_PasteString(R(srch)->editor,newStr);
      numReplaced = 1;
    }

  return numReplaced;
}




/*ARGSUSED*/

int srch_ReplaceTree(Searcher srch, Boolean dir, Boolean global, Boolean all, 
                     FortTreeNode newNode)
{
  /* ... */
  return 0; /* make lint happy */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void selectionToStart(Searcher srch, int line, int sel1, int sel2, Boolean dir, 
                      Boolean zero, int *startLine, int *startCol)
{
  if( line == UNUSED )
    { *startLine = (dir == FRD_FORWARD  ?  sel2+1  :  sel1-1);
      *startCol  = firstOfLine(R(srch)->editor,dir,line);
    }
  else
    { *startLine = line;
      *startCol  = (dir == FRD_FORWARD  ?  sel2+1  :  sel1-1);
    }

  /* kludge to avoid rematching when pattern can be zero-length */
    if( zero  &&  sel2 == sel1-1 )
      *startCol += (dir == FRD_FORWARD ? 1 : -1);
}




static
Boolean findLimited(Searcher srch, Boolean dir, matchStringFunc matcher)
{
  FortEditor ed = R(srch)->editor;
  int increment = (dir == FRD_FORWARD ? 1 : -1);
  Boolean zero = BOOL( matcher == matchPlaceholder );		/* kludge I guess */
  int selline,sel1,sel2,line,col,matchlen;
  Boolean found,more;

  /* determine starting point for search */
    ed_GetSelection(ed,&selline,&sel1,&sel2);
    selectionToStart(srch,selline,sel1,sel2,dir,zero,&line,&col);

  /* search each line in turn until found or no more lines */
    found = false;
    more  = true;
    while( ! found  &&  more )
      { if( moreColumns(ed,dir,line,col) )
          found = searchLine(srch, matcher, dir, line, &col, &matchlen);
        else
          { line += increment;
            more = moreLines(ed,dir,&line);
            if( more )
              col = firstOfLine(ed,dir,line);
          }
      };

  /* set the selection appropriately */
    if( found )
      { if( dir == FRD_FORWARD )
          { /* if forward, 'searchLine' leaves 'col' on first char of match */
            sel1 = col;
            sel2 = sel1 + (matchlen - 1);
          }
        else
          { /* if backward, 'searchLine' leaves 'col' on last char of match */
            sel2 = col;
            sel1 = sel2 - (matchlen - 1);
          }
        ed_SetSelection(ed,line,sel1,sel2);
      }

  return found;
}




static
int firstOfLine(FortEditor ed, Boolean dir, int line)
{
  return (dir == FRD_FORWARD  ?  0  :  ed_GetLineLength(ed,line)-1);
}




static
int lastOfLine(FortEditor ed, Boolean dir, int line)
{
  /* NB -- returns a column number PAST end of line */
  return (dir == FRD_FORWARD  ?  ed_GetLineLength(ed,line)  :  -1);
}




static
Boolean moreLines(FortEditor ed, Boolean dir, int *line)
{
  int numLines = ed_NumLines(ed);
  Boolean more;

  if( ! srch_limit.wrapped )
    { more = BOOL(dir == FRD_FORWARD ? *line <= numLines-1 : *line >= 0);
      if( ! more  &&  srch_limit.wrap )
        { srch_limit.wrapped = true;
          more  = true;
          *line = (dir == FRD_FORWARD ? 0 : numLines-1);
        }
    }
  else
    more = BOOL(dir == FRD_FORWARD ? *line <= srch_limit.line
                                   : *line >= srch_limit.line);

  return more;
}




static
Boolean moreColumns(FortEditor ed, Boolean dir, int line, int col)
{
  int limit;
  Boolean more;

  if( dir == FRD_FORWARD )
    { limit = ed_GetLineLength(ed,line);
      if( obeyLimit(line) )  limit = min(limit,srch_limit.col);
      more = BOOL( col < limit );
    }
  else
    { limit = -1;
      if( obeyLimit(line) )  limit = max(limit,srch_limit.col);
      more = BOOL( col > limit);
    }

  return more;
}




static
Boolean searchLine(Searcher srch, matchStringFunc matcher, Boolean dir, int line, 
                   int *col, int *matchlen)
{
  FortEditor ed = R(srch)->editor;
  Boolean fold = R(srch)->textPatFold;
  Boolean found;
  TextString text;
  int k,last;

  if( R(srch)->textPattern )
    { ed_GetLine(ed,line,&text);

      /* search the line */
        found = false;
        k = *col;
        last = (obeyLimit(line) ? srch_limit.col : lastOfLine(ed,dir,line));

        if( dir == FRD_FORWARD )
          while( ! found  &&  k <= last )
            if( matcher(srch, text, k, last, FRD_FORWARD, fold, matchlen) )
              found = true;
            else
              k += 1;
        else
          while( ! found  &&  k >= last )
            if( matcher(srch, text, k, last, FRD_BACKWARD, fold, matchlen) )
              found = true;
            else
              k -= 1;

      *col = (found ? k : last);

      destroyTextString(text);
    }

  else
    { /* TEMPORARY -- no tree search yet */
      found = false;
      *col = lastOfLine(R(srch)->editor,dir,line);
    }

  return found;
}




static
Boolean matchString(Searcher srch, TextString text, int pos, int last, 
                    Boolean dir, Boolean fold, int *matchlen)
{
  char * str = R(srch)->textPatString;
  int len = strlen(str);
  Boolean match;
  int k;

  /* see whether there is room for a match */
    if( dir == FRD_FORWARD )
      match = BOOL( pos + (len-1) < last );
    else
      { pos = pos - (len-1);
        match = BOOL( pos > last );
      }

  /* check for a match if so */
    if( match )
      { k = 0;
        while( match && k < len )
          { if( fold )
              match = BOOL( match && to_lower(str[k]) == to_lower(text.tc_ptr[pos+k].ch) );
            else
              match = BOOL( match && str[k] == text.tc_ptr[pos+k].ch );
            k += 1;
          }
      }

  *matchlen = len;
  return match;
}




static
Boolean matchPlaceholder(Searcher srch, TextString text, int pos, int last, 
                         Boolean dir, Boolean fold, int *matchlen)
{
  Boolean ph,match;
  int m;
  TextChar tc;
# define PH(tc)    BOOL( tc.style & ftt_PLACEHOLDER_STYLE )

  /* return true if a ph char, or end of stmt #, or start of blank line */

  ph = PH(text.tc_ptr[pos]);
  if( ph )
    match = true;
  else if( pos <= 5 )
    { /* return true if 0..pos-1 are all nonblank non-PH chars */
      match = true;
      for( m = 0;  m < pos;  m++ )
        { tc = text.tc_ptr[m];
          match = BOOL( match && tc.ch != ' ' && NOT( PH(tc) ) );
        }
    }
  else if( pos == text.num_tc )
    { /* return true if 6..pos-1 are blank */
      match = true;
      for( m = 6;  m <= pos-1;  m++ )
        { tc = text.tc_ptr[m];
          match = BOOL( match && tc.ch == ' ' );
        }
    }
  else
    match = false;

  *matchlen = (ph ? 1 : 0);
  return match;
}




static
void setLimit(int line, int col, Boolean wrap)
{
  srch_limit.line    = line;
  srch_limit.col     = col;
  srch_limit.wrap    = wrap;
  srch_limit.wrapped = false;
}




static
Boolean obeyLimit(int line)
{
  return BOOL( line == srch_limit.line  &&  srch_limit.wrapped );
}
