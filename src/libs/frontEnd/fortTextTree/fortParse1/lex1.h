/* $Id: lex1.h,v 1.4 1997/03/11 14:29:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTextTree/FortParse1/lex1.h				*/
/*									*/
/*	FortParse1/lex1 -- scanner for low-level Fortran parser		*/
/*	Last edited: October 21, 1987 at 11:24 pm			*/
/*									*/
/************************************************************************/


#ifndef lex1_h
#define lex1_h

#include <libs/graphicInterface/oldMonitor/include/mon/font.h>

/************************/
/*  Initialization	*/
/************************/

EXTERN(void,		lx1_Init, (void));
EXTERN(void,		lx1_Fini, (void));




/************************/
/*  Scanning		*/
/************************/

EXTERN(void,	lx1_SetScan, (TextString text));
EXTERN(int,	lx1_NextToken, (void));




/************************/
/*  Scaning control	*/
/************************/


extern Boolean	lx1_NeedKwd;
extern Boolean	lx1_IntOnly;
extern Boolean	lx1_InIOControl;
EXTERN(void,    lx1_Flush, (void));




/************************/
/*  Scanner values	*/
/************************/


extern char	lx1_Token [ ];
extern int	lx1_TokenLen;
extern long int	lx1_StatNumber;
extern Boolean	lx1_PlaceholderStatLabel;
extern KWD_OPT	lx1_IOKwd;
EXTERN(char*,	lx1_GetLine, (int *n));



EXTERN (int, hextoi, (int c));
EXTERN (Boolean, eqn, (int n, char *a, char *b));

#endif







