/* $Id: SymTrans.h,v 1.4 1997/03/11 14:35:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef SymTrans_h
#define SymTrans_h

#include <libs/moduleAnalysis/valNum/val_enum.h>

struct ValIP;	/* minimal definition */
struct ValTable;
struct CallGraphNode;
struct CallGraphEdge;

typedef FUNCTION_POINTER(ValNumber, SymTransCallback,
                         (void *handle, CallGraphEdge *edge,
			  char *name, int offset, int length));

EXTERN(struct SymValAnnot *, SymExploitMod, (CallGraphNode *node));

EXTERN(ValNumber, SymTranslate, (CallGraphEdge *edge, ValTable *newTab, 
				 ValNumber ov,
				 SymTransCallback transFn, void *handle));

EXTERN(ValNumber, trans_ret_cfgval, (void *v_cg, ValTable *V, 
				     char *proc, int site,
				     char *name, int offset, int length, 
				     void *cfg));

EXTERN(ValNumber, cfgval_translate_passed, (void *cfg, int site, char *name, 
					    int offset, int length));

#endif
