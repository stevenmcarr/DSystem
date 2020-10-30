/* $Id: FortTextTree.i,v 1.6 */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FortTextTree_i
#define FortTextTree_i

/************************************************************************/
/*									*/
/*	FortTextTree/FortTextTree.i					*/
/*									*/
/*	FortTextTree -- text-and-structure view of a FortTree		*/
/*									*/
/************************************************************************/




#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>



#define FIXED_INDENT       6


/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    /* contents */
      FortTree	         ft;
      TextTree	         tt;
      FortTreeSideArray  sideArray;
      Flex               *expansions;

  } ftt_Repr;

#define	R(ob)		((ftt_Repr *) ob)


extern TT_Methods ftt_methods;


/************************/
/*  Statement Tokens	*/
/************************/


typedef struct
  {
    int token;
    FortTreeNode tree;
    FortTreeNode part[4];

    int conceal;
    TT_LineTag tt_tag;

  }  fx_StatToken;


typedef struct
  {
    Boolean missing;
    FortTreeNode label;

    int conceal;
    TT_LineTag tt_tag;

  }  fx_EndingStat;


extern fx_EndingStat UNUSED_ENDVAL;





/* this reveals too much -- only want the _STAT token constants */

#ifndef gram2_h
#define gram2_h
#include <libs/frontEnd/fortTextTree/fortParse2/gram2.h>
#endif

EXTERN(Boolean, ftt_nodeIsLevel2, (FortTreeNode node));
EXTERN(int, ftt_nodeTypeToTokenType, (NODE_TYPE nt));
EXTERN(char *, ftt_getTokenFormat, (int tok));
EXTERN(char *, ftt_getNodeFormat, (NODE_TYPE nt));


extern FortTree			ftt_fortTree;
extern TextTree			ftt_textTree;




/************************/
/*  Parsing errors	*/
/************************/




/* bracketing errors */

#define	ftt_NO_ERROR		0
#define	ftt_MISSING_ENDBRACKET	1
#define	ftt_WRONG_ENDBRACKET	2


/* Note: for more complicated errors, the ParseErrorCode	*/
/* will be the statement token value.				*/




/************************/
/*  Expanding		*/
/************************/


typedef struct
  {
    FortTreeNode node;
    Generic value;
    int type;
    TextString title;
    short who;
    Boolean first;

  }  fx_Expansion;


/* "who" values */

#define PLACEHOLDER_EXPANDER	0
#define UNPARSE1_EXPANDER	1
#define UNPARSE2_EXPANDER	2




/************************/
/*  Unparsing		*/
/************************/

typedef FUNCTION_POINTER (void,OutputFunc,(Generic,char));
typedef FUNCTION_POINTER (void,OutputTextFunc,(Generic));
typedef FUNCTION_POINTER (void,OutputSonFunc,(Generic,char));
typedef FUNCTION_POINTER (void,TabFunc,(Generic));
typedef FUNCTION_POINTER (Boolean,EvalFunc,(Generic,char *,int));
typedef FUNCTION_POINTER (void,MissingFunc,(Generic,char *,int,int));


typedef struct
  {
    OutputFunc output;
    OutputTextFunc outputText;
    OutputSonFunc outputSon;
    TabFunc tab;
    EvalFunc eval;
    MissingFunc missing;
  }  ftt_FormatCustomProcs;


EXTERN(void,ftt_format,(char *fmt, Generic ob, ftt_FormatCustomProcs *custom ));

EXTERN(void,ftt_toggleBold,(void));
EXTERN(void,ftt_toggleItalic,(void));
EXTERN(void,ftt_toggleReverse,(void));
EXTERN(void,ftt_toggleHalf,(void));
EXTERN(void,ftt_toggleUnderline,(void));

EXTERN(TextString,ftt_expansionName,(int type));
EXTERN(TextString,ftt_makeExpansionName,(char* name, Boolean ph_default));

extern unsigned char		ftt_style;
#endif
