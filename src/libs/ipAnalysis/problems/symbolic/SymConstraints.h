/* $Id: SymConstraints.h,v 1.1 1997/03/11 14:35:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SymConstraints_h
#define SymConstraints_h

#include <libs/moduleAnalysis/valNum/val_enum.h>

struct ValIP;	/* minimal definition */
struct CallGraphNode;
struct SymConAnnot;

EXTERN(ValIP *,       SymGetEntryCfgVals, (void *passed_cg, char *name));
EXTERN(ValIP *,       SymGetEntryVals,    (CallGraphNode *node));
EXTERN(void,          SymFwdEdge,         (CallGraphNode *node));
EXTERN(SymConAnnot *, SymFwdMerge,        (CallGraphNode *node));

#endif
