/* $Id: TypeChecker.h,v 1.7 1997/03/11 14:29:54 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef TypeChecker_h
#define TypeChecker_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/fortTree/FortTree.h>

/* main entry points */
EXTERN(void, FortTreeTypeCheck, (TableDescriptor td, FortTree ft));
EXTERN(void, FortStmtTypeCheck, (TableDescriptor td, AST_INDEX stmt));

/* support routines */
EXTERN(Boolean, isConstantExpr, (SymDescriptor d, AST_INDEX node));
EXTERN(Boolean, isExecutable, (AST_INDEX node));
EXTERN(Boolean, isDeclaration, (AST_INDEX node));
EXTERN(void, StompTypes, (FortTree ft));

#endif 
