/* $Id: TypeChecker.h,v 1.8 1997/06/24 17:53:08 carr Exp $ */
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
EXTERN(void, ConstantSetType,(AST_INDEX node));

#endif 
