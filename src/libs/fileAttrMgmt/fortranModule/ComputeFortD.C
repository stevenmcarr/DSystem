/* $Id: ComputeFortD.C,v 1.1 1997/03/11 14:27:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ComputeFortD.C
//
// Author: Gil Hansen						April 1994
//
// Copyright 1994, Rice University
//***************************************************************************

#include <assert.h>

#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/database/context.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/fortD/misc/FortD.h>
#include <libs/frontEnd/fortTree/InitInfo.h>
#include <libs/frontEnd/fortTree/ft.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/ipAnalysis/ipInfo/ProcFortDInfo.h>

ProcLocalInfo *ComputeFortDInfo(FortTree ft, AST_INDEX proc)
{
  ProcFortDInfo *summary;

  summary = new ProcFortDInfo(gen_get_text(get_name_in_entry(proc)));
  summary->tree = NULL;

  ast_select(ft->asttab);

  if (ft->state == ft_CORRECT)
  {
    // compute initial Fortran D interprocedural information

    /* NOTE: context saved in LInfo & passed to FortranDInfo by
       FortranDProblem::ComputeLocalInfo() */
    LInfo *LocInfo = new LInfo(ft, (Context)0);

    FortranDProblem *m = new FortranDProblem();
    LocInfo->p =  m;

    /* Generate local Fortran D info for procedure.
       NOTE: 1) ultimately call FortranDProblem::ComputeLocalInfo()
                which initiates the Fortran D information problem
	     2) FortranDProblem::ComputeLocalInfo() ultimately calls
		FortranDProblem::WalkRoutine() which traverses the
		procedure and collects Fortran D decomposition, common
		block, and overlap information
     */
    
    /* Incorporate code found in call chain InitialInfo(), WalkInfoProc()
       WalkLocalInof() */
    // code from InitialInfo()
    /* it is assumed get_get_node_type(proc) is GEN_FUNCTION, GEN_SUBROUTINE */

    // code from WalkInfoProc()
    AST_INDEX stmt_list;
    LocInfo->I = new ModRefNameTreeNode(SEQBLOCK,
			ft_NodeToNumber(LocInfo->ft, proc), 0);

    LocInfo->proc_sym_table = fst_GetTable(LocInfo->ft->td,
			      gen_get_text(get_name_in_entry(proc)));
    assert(LocInfo->proc_sym_table != 0);
    LocInfo->fd = new FortranDInfo();
    LocInfo->node = proc;
    stmt_list = get_stmts_in_scope(proc);

    // code from WalkLocalInfo()
    LocInfo->p->ComputeLocalInfo(stmt_list, (Generic)LocInfo);

    /* NOTE: Fortran D information contained in the root node of the iptree
       and in the callsite list */
    assert (LocInfo->ipt != 0);
    summary->tree = LocInfo->ipt;
  }
  return summary;
}

