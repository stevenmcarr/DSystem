/* $Id: FortParse2.i,v 1.5 1997/06/24 17:47:33 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTexTree/FortParse2/FortParse2.i			*/
/*									*/
/*	FortParse2 -- high-level parser for Fortran source		*/
/*	Last edited: August 19, 1992 at 12:23 pm			*/
/*									*/
/************************************************************************/




#ifndef gram2_h
#define gram2_h
#include <libs/frontEnd/fortTextTree/fortParse2/gram2.h>
#endif


#include <libs/frontEnd/fortTextTree/FortParse2.h>





/************************/
/* Parser results	*/
/************************/




extern FortTreeNode fp2_root;





/************************/
/* Scanner stuff	*/
/************************/


EXTERN(void,lx2_Init,(void));
EXTERN(void,lx2_Fini,(void));
EXTERN(void,lx2_SetScan,(int goal, Flex *lines, int start, int count));

EXTERN(int,yy2lex,(void));
EXTERN(int,yy2parse,(void));
EXTERN(void,yy2error,(char*));
