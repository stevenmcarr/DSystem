/* $Id: CollectLocalInfo.h,v 1.1 1997/03/11 14:27:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// CollectLocalInfo.i
//
// Author: John Mellor-Crummey                                August 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef CollectLocalInfo_i
#define CollectLocalInfo_i

#include <libs/frontEnd/fortTree/FortTree.h>

class ModuleLocalInfo;    // minimal external declaration
class ProcLocalInfo;      // minimal external declaration

typedef ProcLocalInfo *(*CollectProcLocalInfoCallBack) (FortTree, AST_INDEX);

int CollectLocalInfo(FortTree ft, ModuleLocalInfo *info, 
		     CollectProcLocalInfoCallBack collect);

#endif
