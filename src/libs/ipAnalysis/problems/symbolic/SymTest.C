/* $Id: SymTest.C,v 1.2 1997/03/11 14:35:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>

EXTERN(void, SymTest, (C_CallGraph ccg));

void SymTest(C_CallGraph ccg)
{
    CallGraph *cg = (CallGraph *) ccg;

    CallGraphNode *prog = (CallGraphNode *)cg->GetRoot();

    SymValAnnot *newSym = (SymValAnnot *) prog->GetAnnotation(SYMVAL_ANNOT, 
							      true);

    val_Dump(newSym->valIp()->values);
}
