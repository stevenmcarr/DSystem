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
    int value;
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


typedef struct
  {
    PFV output;
    PFV outputText;
    PFV outputSon;
    PFV tab;
    PFB eval;
    PFV missing;
  }  ftt_FormatCustomProcs;


extern void			ftt_format(/* fmt, ob, custom */);

extern unsigned char		ftt_style;
extern void			ftt_toggleBold(void);
extern void			ftt_toggleItalic(void);
extern void			ftt_toggleReverse(void);
extern void			ftt_toggleHalf(void);
extern void			ftt_toggleUnderline(void);

extern TextString		ftt_expansionName(int type);
extern TextString		ftt_makeExpansionName(char* name, Boolean ph_default);

#endif
