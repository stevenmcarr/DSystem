/* $Id: AddressOptimization.C,v 1.5 2002/02/20 16:17:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

#include <iostream.h>
#include <libs/support/misc/general.h>
#include <libs/Memoria/include/mh.h>
#include <libs/Memoria/include/mh_ast.h>
#include <libs/Memoria/include/mh_config.h>
#include <libs/Memoria/include/label.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/Memoria/include/bound.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/Memoria/include/header.h>
#include <libs/Memoria/include/mark.h>
#include <libs/Memoria/include/analyze.h>
#include <libs/Memoria/include/mem_util.h>
#include <libs/Memoria/annotate/CacheAnalysis.h>
#include <libs/Memoria/include/la.h>
#include <libs/Memoria/annotate/AddressEquivalenceClassSet.h>

static void UpdateAddressInfoSubscript(AST_INDEX Id,
				       ReferenceAccessType aType,
				       va_list args)
{
  AST_INDEX node;
  SymDescriptor Symtab;

  Symtab = va_arg(args,SymDescriptor);
  if (fst_GetField(Symtab,gen_get_text(Id),SYMTAB_NUM_DIMS) > 0 && 
      (aType == at_ref || aType == at_mod))
    {
      node = tree_out(Id);
      if (DepInfoPtr(node) == NULL)
	CreateDepInfoPtr(node);
      DepInfoPtr(node)->AddressLeader = AST_NIL;
      DepInfoPtr(node)->FirstInLoop = AST_NIL;
      DepInfoPtr(node)->Offset = -1;
    }
}


static int CreateAddressInfo(AST_INDEX node,
			     int       level,
		             SymDescriptor Symtab)

  {

    if (is_comment(node))
      if (DirectiveInfoPtr(node) != NULL)
	{
	  DirectiveInfoPtr(node)->AddressLeader = AST_NIL;
	  DirectiveInfoPtr(node)->FirstInLoop = AST_NIL;
	  DirectiveInfoPtr(node)->Offset = -1;
	}
      else;
    else
      walkIDsInStmt(node,(WK_IDS_CLBACK_V)UpdateAddressInfoSubscript,Symtab);
    return(WALK_CONTINUE);
  }


static int StoreAddressInfo(AST_INDEX     node,
			    CacheInfoType *CacheInfo)
			  
  {
     if (is_subscript(node))
       {
	 DepInfoPtr(node)->AddressLeader = CacheInfo->AECS->GetLeader(node);
	 DepInfoPtr(node)->FirstInLoop = CacheInfo->AECS->GetFirstInLoop(node);
	 DepInfoPtr(node)->Offset = CacheInfo->AECS->GetOffset(node);
       }
     else if (is_comment(node))
       if (DirectiveInfoPtr(node) != NULL)
	 if (DirectiveInfoPtr(node)->Instr != Cluster)
	   {
	     DirectiveInfoPtr(node)->AddressLeader = 
	       CacheInfo->AECS->GetLeader(DirectiveInfoPtr(node)->Subscript);
	     DirectiveInfoPtr(node)->FirstInLoop = 
	       CacheInfo->AECS->GetFirstInLoop(DirectiveInfoPtr(node)->Subscript);
	     DirectiveInfoPtr(node)->Offset = 
	       CacheInfo->AECS->GetOffset(DirectiveInfoPtr(node)->Subscript);
	 }
     return(WALK_CONTINUE);
  }


static void walk_loops(CacheInfoType  *CacheInfo,
		       int            loop)
		       

  {
   int i;

     CacheInfo->IVar[CacheInfo->loop_data[loop].level-1] = 
          gen_get_text(gen_INDUCTIVE_get_name(gen_DO_get_control(
                       CacheInfo->loop_data[loop].node)));
     if (CacheInfo->loop_data[loop].inner_loop == -1)
       {
	CacheInfo->loop = loop;
	CacheInfo->AECS = 
	  new AddressEquivalenceClassSet(CacheInfo->loop_data[loop].node,
					 CacheInfo->loop_data[loop].level,
					 CacheInfo->IVar);
	walk_expression(CacheInfo->loop_data[loop].node,(WK_EXPR_CLBACK)StoreAddressInfo,
			NOFUNC,(Generic)CacheInfo);
	delete CacheInfo->AECS;
       }
     else
       {
	i = CacheInfo->loop_data[loop].inner_loop;
	while(i != -1)
	  {
	   walk_loops(CacheInfo,i);
	   i = CacheInfo->loop_data[i].next_loop;
	  }
       }
  }


void memory_PerformAddressOptimization(PedInfo      ped,
				       SymDescriptor symtab,
				       arena_type   *ar,
				       AST_INDEX    root,
				       int          level,
				       int          loop_num)

/****************************************************************************/
/*                                                                          */
/*                                                                          */
/****************************************************************************/

  {
   CacheInfoType  CacheInfo;

     CacheInfo.loop_data = (model_loop *)ar->arena_alloc_mem_clear
                  (LOOP_ARENA,loop_num*sizeof(model_loop)*4);
     ut_analyze_loop(root,CacheInfo.loop_data,level,ped,symtab);

     walk_statements(root,level,(WK_STMT_CLBACK)CreateAddressInfo,NOFUNC,
		     (Generic)symtab);
     CacheInfo.ped = ped;
     CacheInfo.IVar = new char*[loop_num];
     CacheInfo.symtab = symtab;
     CacheInfo.ar = ar;
     walk_loops(&CacheInfo,0);
     delete CacheInfo.IVar;
  }
