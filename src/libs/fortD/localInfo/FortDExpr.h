/* $Id: FortDExpr.h,v 1.8 1997/03/11 14:28:40 carr Exp $ */
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

#include <sys/stream.h>
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

#endif fd_expr_h
