/* $Id: Expander.h,v 1.5 1997/03/11 14:30:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortEditor/Expander.h					*/
/*									*/
/*	Expander -- chooses possible expansions for a FortEditor	*/
/*	Last edited: March 27, 1989 at 4:45 pm				*/
/*									*/
/************************************************************************/




#ifndef Expander_h
#define Expander_h


typedef Generic Expander;




/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		ex_Init,(void));
EXTERN(void,		ex_Fini,(void));

EXTERN(Expander,	ex_Open,(Context context, DB_FP *fd, FortEditor ed, 
                                 FortTextTree ftt));
EXTERN(void,		ex_Close,(Expander ex));
EXTERN(void,		ex_Save,(Expander ex, Context context, DB_FP *fd));





/************************/
/*  Expansions		*/
/************************/

EXTERN(Boolean,		ex_Expandee,(Expander ex));
EXTERN(Boolean,		ex_AskChoice,(Expander ex, int *choice));
EXTERN(FortTreeNode,	ex_Expand,(Expander ex, int choice));




/************************/
/*  Keyboard Shorthand	*/
/************************/

EXTERN(Boolean,		ex_Key,(Expander ex, char ch));
EXTERN(void,		ex_Select,(Expander ex, Boolean placeholder));
EXTERN(Boolean,		ex_Other,(Expander ex));
EXTERN(Boolean,		ex_IsShorthand,(Expander ex));




#endif
