/* $Id: FortParse1.i,v 1.5 1997/03/11 14:29:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/FortTexTree/FortParse1/FortParse1.i			*/
/*									*/
/*	FortParse1 -- low-level parser for Fortran source		*/
/*	Last edited: August 19, 1992 at 12:21 pm			*/
/*									*/
/************************************************************************/

#ifndef FortParse1_i
#define FortParse1_i


#include <libs/frontEnd/fortTextTree/FortParse1.h>




/************************/
/*  Parsing information	*/
/************************/


extern fx_StatToken	fp1_token;
extern char *		fp1_error;
EXTERN(void,		parse1, (FortTextTree ftt, TextString text));
EXTERN(void,		yy1error, (char *s));




/************************/
/*  IO keyword values	*/
/************************/


typedef enum
  {
    IOUNIT, IOFMT, IOERR, IOEND, IOIOSTAT, IOREC, IORECL, IOFILE,
    IOSTATUS, IOACCESS, IOFORM, IOBLANK, IOEXIST, IOOPENED, IONUMBER, IONAMED,
    IONAME, IOSEQUENTIAL, IODIRECT, IOFORMATTED, IOUNFORMATTED, IONEXTREC,
    IOERROR
  } KWD_OPT;




/************************/
/*  Token values	*/
/************************/


#ifndef gram1_h
#define gram1_h
#include <libs/frontEnd/fortTextTree/fortParse1/gram1.h>
#endif




/************************/
/*  Tree stack routines	*/
/************************/


EXTERN(void, treeStackDrop, (int num));
EXTERN(void, treeStackPush, (FortTreeNode node));
EXTERN(void, treeStackCheck, (void));

#endif
