/* $Id: Expander.C,v 1.1 1997/06/25 13:43:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/Expander.c					*/
/*									*/
/*	Expander -- chooses possible expansions for a FortEditor	*/
/*	Last edited: October 25, 1989 at 1:12 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>

#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/Expander.h>

#include <include/bstring.h>

#include <ctype.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef enum
  {
    kb_neutral,
    kb_preExpand,
    kb_canExpand,
    kb_didExpand					/* not used yet */

  } KB_State;

#define MAX_SHORTHAND	80



typedef struct
  {
    FortEditor		ed;
    FortTextTree	ftt;
    int			count;

    KB_State		kbState;
    Selection		kbSelection;
    int			kbCurLine;
    TextString		kbCurText;
    Boolean		kbDirty;
    char		kbString[MAX_SHORTHAND];
    int			kbChoice;			/* not used yet */

  } ex_Repr;


#define	R(ob)		((ex_Repr *) ob)






/*************************/
/* Temporary choice list */
/*************************/




#define MAX_CHOICES    99

static TextString ex_nameList[MAX_CHOICES];
static int        ex_numList [MAX_CHOICES];
static int        ex_count;






/***********************/
/* Stuff for matching  */
/***********************/




typedef enum
  {
    MATCH_NONE,
    MATCH_SOME,
    MATCH_ALL

  } MatchKind;






/************************/
/* Forward declarations	*/
/************************/




STATIC(void,		chooseKbExpansion,(Expander ex, int *choice));
STATIC(int,		matchChoice,(TextString choice, char *pattern));
STATIC(void,		tc_tokenLength,(TextChar *tc, int maxlen, int *len1, int *len2));
STATIC(void,		s_tokenLength,(char *s, int maxlen, int *len1, int *len2));
STATIC(MatchKind,	matchToken,(TextChar *tc, int tc_len, char *s, int s_len));
STATIC(short,		skw_menu_select,(char *title, short num, TextString labels[]));
STATIC(Boolean,		justOneFirst,(Expander ex));
STATIC(Boolean,		choicePresentIn,(int first, int last, TextString name));







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void ex_Init()
{
  /* nothing */
}




void ex_Fini()
{
  /* nothing */
}




/*ARGSUSED*/

Expander ex_Open(Context context, DB_FP *fp, FortEditor ed, FortTextTree ftt)
{
  Expander ex;

  /* allocate a new instance */
    ex = (Expander) get_mem(sizeof(ex_Repr),"FortEditor:Expander");
    if( (Generic) ex == 0 ) return UNUSED;

  /* initialize the parts */
    /* set creation parameters */
      R(ex)->ed = ed;
      R(ex)->ftt = ftt;
      R(ex)->count = 0;

    /* initialize kb shorthand state machine */
      R(ex)->kbState = kb_neutral;

  return (Generic) ex;
}




void ex_Close(Expander ex)
{
  free_mem((void*) ex);
}




/*ARGSUSED*/

void ex_Save(Expander ex, Context context, DB_FP *fp)
{
  /* nothing */
}







/************************/
/*  Expansions		*/
/************************/




Boolean ex_Expandee(Expander ex)
{
  FortEditor ed = R(ex)->ed;
  Selection sel;

  if( R(ex)->kbState == kb_canExpand )
    { ed__SetStatus(ed, R(ex)->kbSelection,
                        R(ex)->kbCurLine,
                        R(ex)->kbCurText,
                        R(ex)->kbDirty);
      destroyTextString(R(ex)->kbCurText);
      ed__MakeClean(ed,true,true,true,&sel);
    }
  else
    ed__MakeClean(ed,true,true,false,&sel);

  if( sel.structure )
    { /* structure expansion */
       R(ex)->count = ftt_NodeExpandee(R(ex)->ftt, sel.node);
    }
  else if( sel.line != UNUSED   &&  sel.sel1 > sel.sel2 )
    { /* insertion point expansion */
       R(ex)->count = ftt_IPExpandee(R(ex)->ftt, sel.line, sel.sel1);
    }
  else
    { /* nothing here */
       R(ex)->count = 0;
    }
  return BOOL(R(ex)->count);
}




Boolean ex_AskChoice(Expander ex, int *choice)
{
  Boolean wantExpand;

  if( R(ex)->count == 0 )
    wantExpand = false;
  else if( R(ex)->kbState == kb_canExpand )
    { chooseKbExpansion(ex, choice);
      wantExpand = BOOL( *choice != UNUSED );
    }
  else if( justOneFirst(ex) )
    { *choice = 0;
      wantExpand = true;
    }
  else
    { ex_count = ftt_GetExpansionNames(R(ex)->ftt, true, ex_nameList, ex_numList);

      *choice = skw_menu_select("expansions:", ex_count, ex_nameList);
      if( *choice != UNUSED )  *choice = ex_numList[*choice];
      wantExpand = BOOL( *choice != UNUSED );
    }
  
  R(ex)->kbState = kb_neutral;

  return wantExpand;
}




FortTreeNode ex_Expand(Expander ex, int choice)
{
  FortTreeNode New,focus;
  Boolean go;

  ftt_Expand(R(ex)->ftt, choice, &New, &focus);
  ed_SetSelectedNode(R(ex)->ed,focus);

  go = true;
  while( go && focus == New && ex_Expandee(ex) )
    if( ex_AskChoice(ex, &choice) )
      { ftt_Expand(R(ex)->ftt, choice, &New, &focus);
        ed_SetSelectedNode(R(ex)->ed,focus);
      }
    else
      go = false;

  return focus;
}






/************************/
/*  Keyboard Shorthand	*/
/************************/




Boolean ex_Key(Expander ex, char ch)
{
  FortEditor ed = R(ex)->ed;
  int len;

  switch( R(ex)->kbState )
    {
      case kb_neutral:
        break;

      case kb_preExpand:
        if( ch == KB_Backspace )
          { /* nothing */ }
        else
          { /* save the current status */
              /* ASSERT: selection is contained on a single line */
              ed__GetStatus(ed, &R(ex)->kbSelection,
                               &R(ex)->kbCurLine,
                               &R(ex)->kbCurText,
                               &R(ex)->kbDirty);

            R(ex)->kbString[0] = ch;
            R(ex)->kbString[1] = '\0';
            R(ex)->kbState = kb_canExpand;
          }
        break;

      case kb_canExpand:
        len = strlen(R(ex)->kbString);
        if( ch == KB_Backspace )
          { if( len > 0 )
              R(ex)->kbString[len-1] = '\0';
          }
        else
          { R(ex)->kbString[len]   = ch;
            R(ex)->kbString[len+1] = '\0';
          }
        break;

      case kb_didExpand:
        R(ex)->kbState = kb_neutral;
        break;

    }

  return BOOL( R(ex)->kbState == kb_canExpand );
}




void ex_Select(Expander ex, Boolean placeholder)
{
  Selection sel;
  
  /* remove any previous shorthand hilighting */
    if( R(ex)->kbState == kb_canExpand )
      ed__MakeClean(R(ex)->ed,false,placeholder,true,&sel);

  if( placeholder )
    { R(ex)->kbState = kb_preExpand;
      R(ex)->kbSelection = sel;
    }
  else
    R(ex)->kbState = kb_neutral;
}




Boolean ex_Other(Expander ex)
{
  Boolean wasActive = NOT( R(ex)->kbState == kb_neutral );

  R(ex)->kbState = kb_neutral;

  return wasActive;
}




Boolean ex_IsShorthand(Expander ex)
{
  return BOOL( R(ex)->kbState == kb_canExpand );
}








/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void chooseKbExpansion(Expander ex, int *choice)
{
  int maxScore,maxCount,maxChoice,j,k;
  int score[MAX_CHOICES];

  ex_count = ftt_GetExpansionNames(R(ex)->ftt, false, ex_nameList, ex_numList);

  /* find choices tied for best match to the kb shorthand */
    maxScore = 0;
    maxCount = 0;
    for( k = 0;  k < ex_count;  k++ )
      { score[k] = matchChoice(ex_nameList[k], R(ex)->kbString);
        if( score[k] > maxScore )
          { maxScore  = score[k];
            maxCount  = 1;
            maxChoice = k;
          }
        else if( score[k] == maxScore )
          maxCount += 1;
      }

  /* determine the winning choice if any */
    if( maxScore == 0 )
      *choice = UNUSED;
    else if( maxCount == 1 )
      *choice = ex_numList[maxChoice];
    else
      { /* prune the choice list to include only top scorers, removing dups */
          j = 0;
          for( k = 0;  k < ex_count;  k++ )
            if( score[k] == maxScore )
              if( ! choicePresentIn(0, j-1, ex_nameList[k]) )
                { ex_nameList[j] = ex_nameList[k];
                  ex_numList [j] = ex_numList [k];
                  j += 1;
                }
          ex_count = j;

        if( ex_count == 1 )
          *choice = ex_numList[0];
        else
          { /* let the user choose from among the top scorers */
              *choice = skw_menu_select("expansions:", ex_count, ex_nameList);
              if( *choice != UNUSED )  *choice = ex_numList[*choice];
          }
      }
}




static
int matchChoice(TextString choice, char *pattern)
{
  TextChar * c_limit = choice.tc_ptr + choice.num_tc;
  char * p_limit     = pattern + strlen(pattern);
  TextChar * c_token;
  char * p_token;
  int c_len1, c_len2, p_len1, p_len2;
  int score;
  MatchKind match;

  score = 0;

  c_token = choice.tc_ptr;
  p_token = pattern;
  while( c_token < c_limit && p_token < p_limit )
    { tc_tokenLength(c_token, c_limit - c_token, &c_len1, &c_len2);
      s_tokenLength(p_token, p_limit - p_token, &p_len1, &p_len2);
      match = matchToken(c_token, c_len1, p_token, p_len1);
      if( match == MATCH_NONE )
        { score -= 1;
          c_token += c_len2;
        }
      else
        { score += (match == MATCH_ALL ? 11 : 10);
          c_token += c_len2;
          p_token += p_len2;
        }
    }

  return (p_token == p_limit ? score : -9999);
}




static
void tc_tokenLength(TextChar *tc, int maxlen, int *len1, int *len2)
{
  int len;

  if( isalpha(tc[0].ch) )
    { len = 0;
      while( len < maxlen && isalnum(tc[len].ch) )  len += 1;
    }
  else
    len = 1;
  *len1 = len;

  while( len < maxlen && tc[len].ch == ' ' )  len += 1;
  *len2 = len;
}




static
void s_tokenLength(char *s, int maxlen, int *len1, int *len2)
{
  int len;

  if( isalpha(s[0]) )
    { len = 0;
      while( len < maxlen && isalnum(s[len]) )  len += 1;
    }
  else
    len = 1;
  *len1 = len;

  while( len < maxlen && s[len] == ' ' )  len += 1;
  *len2 = len;
}




static
MatchKind matchToken(TextChar *tc, int tc_len, char *s, int s_len)
{
  MatchKind kind;
  Boolean matches;
  int k;

  if( tc_len < s_len )
    kind = MATCH_NONE;
  else
    { matches = true;
      k = 0;
      while( matches && k < s_len )
        if( tc[k].style & ftt_PLACEHOLDER_STYLE )
          matches = false;
        else if( tc[k].ch != s[k] )
          matches = false;
        else
          k += 1;

      if( k < s_len )
        kind = MATCH_NONE;
      else if( k == tc_len )
        kind = MATCH_ALL;
      else
        kind = MATCH_SOME;
    }

  return kind;
}




static
short skw_menu_select(char *title, short num, TextString labels[])
{
  int k,size;
  TextChar * tc_labels[99];
  short choice;

  for( k = 0;  k < num;  k++ )
    { size = (1 + labels[k].num_tc) * sizeof(TextChar);
      tc_labels[k] = (TextChar *) get_mem(size, "skw_menu_select");
      bcopy( (char *) labels[k].tc_ptr, (char *) tc_labels[k], size);
      tc_labels[k][labels[k].num_tc] = makeTextChar('\0',0);
    }

  choice = menu_select(title, num, (char **)tc_labels);

  for( k = 0;  k < num;  k++ )
    free_mem((void*) tc_labels[k]);

  return choice;
}




static
Boolean justOneFirst(Expander ex)
{
  int count = ftt_GetExpansionNames(R(ex)->ftt, true, nil, nil);

  return BOOL( count == 1 );
}




static
Boolean choicePresentIn(int first, int last, TextString name)
{
  int k,m;
  Boolean present, equal;

  present = false;
  k = first;
  while( ! present  &&  k <= last )
    { /* compare 'name' to 'ex_nameList[k]' */
        if( name.num_tc != ex_nameList[k].num_tc )
          equal = false;
        else
          { equal = true;
            m = 0;
            while( equal  &&  m < name.num_tc )
              if( ! equalTextChar(name.tc_ptr[m], ex_nameList[k].tc_ptr[m]) )
                equal = false;
              else
                m += 1;
          }

      if( equal )
        present = true;
      else
        k += 1;
    }

  return present;
}
