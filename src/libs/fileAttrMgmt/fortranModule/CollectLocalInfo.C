/* $Id: CollectLocalInfo.C,v 1.1 1997/03/11 14:27:53 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// CollectLocalInfo.C
//
// Author: John Mellor-Crummey                                December 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#include <libs/fileAttrMgmt/fortranModule/CollectLocalInfo.h>
#include <libs/ipAnalysis/ipInfo/ModuleLocalInfo.h>


int CollectLocalInfo(FortTree ft, ModuleLocalInfo *info, 
		     CollectProcLocalInfoCallBack collect)
{
  ft_AstSelect(ft);

  AST_INDEX scopeList = gen_GLOBAL_get_subprogram_scope_LIST(ft_Root(ft));
  
  AST_INDEX elt = list_first(scopeList);
  while (elt != AST_NIL) { 
    switch(gen_get_node_type(elt)) {
    case GEN_FUNCTION:
    case GEN_SUBROUTINE:
    case GEN_PROGRAM:
    case GEN_BLOCK_DATA: {
      ProcLocalInfo *entry = collect(ft, elt);
      if (entry) info->AddProcEntry(entry);
      break;
	  }
    default:
      break;
    }
    elt = list_next(elt);
  }

  return 0;
}

