/* $Id: FortParse2.i,v 1.4 1997/03/11 14:29:44 carr Exp $ */
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


extern	void	lx2_Init();
extern	void	lx2_Fini();
extern	void	lx2_SetScan();
