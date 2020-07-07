/* $Id: FortDExpr.h,v 1.10 1999/06/11 20:35:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fd_expr_h
#define fd_expr_h

#include <string.h>
#include <sys/types.h>
#include <libs/support/misc/general.h>
#include <libs/support/strings/rn_string.h>

#undef is_open

#include <iostream>

#include <libs/frontEnd/include/expr.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/support/file/FormattedFile.h>

EXTERN(void, expr_init, (Expr *e));
EXTERN(void, expr_lower, (Expr *e, Expr_type t, int v));
EXTERN(void, expr_upper, (Expr *e, Expr_type t, int v));
EXTERN(void, expr_upper_2,(Expr *e,Expr_type t, int v, char* s));
EXTERN(void, expr_write, (Expr *e, FormattedFile & port));
EXTERN(void, expr_read, (Expr *e, FormattedFile & port));
EXTERN(void, expr_copy, (Expr *copy_into, Expr *copy_from));
EXTERN(char*,expr_string, (Expr *e));

#endif 
