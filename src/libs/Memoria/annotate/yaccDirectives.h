/* $Id: yaccDirectives.h,v 1.2 1997/03/27 20:22:30 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/


typedef union 
  {
   Instruction ival;
   char       *cval;
   Directive   dval;
   AST_INDEX   aval;
  } A2I_STYPE;
extern A2I_STYPE a2i_lval;
# define CDIR 257
# define PREFETCH 258
# define FLUSH 259
# define NAME 260
# define ICONST 261
# define LPAR 262
# define RPAR 263
# define COMMA 264
# define PLUS 265
# define MINUS 266
# define TIMES 267
# define DIVIDE 268
