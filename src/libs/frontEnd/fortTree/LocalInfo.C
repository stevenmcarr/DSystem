/* $Id: LocalInfo.C,v 1.10 1997/03/11 14:29:50 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

// #include <ned.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>
#include <libs/frontEnd/fortTree/InitInfo.h>

/* C++ includes for mod/ref information */

#include <libs/frontEnd/fortTree/modrefnametree.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/ipAnalysis/ipInfo/module.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>


/* extern declarations */
extern IPinfoTree* loop_tree_info_for_entry
(FortTree ft, AST_INDEX node, SymDescriptor d, ModRefNameTreeNode *I);

extern void LocInfo_proc_sym_table(LInfo *LocInfo, AST_INDEX node);
 
/* forward declarations */

void WalkInitialInformation(AST_INDEX root, LInfo  *LocInfo);

static int InitialInfo(AST_INDEX node, int level, Generic LocInfo);
static void WalkInfoProc(AST_INDEX node, LInfo *LocInfo);
static void WalkLocalInfo(AST_INDEX node, Generic LocInfo);

/**********************************************************************/
/* A variety of initial information for interprocedural analysis      */
/* is needed. FortTree should be designed in a way that allows        */
/* different local information to be collected in a modular way       */
/**********************************************************************/
void WalkInitialInformation(AST_INDEX root, LInfo  *LocInfo)
{
 walk_statements(gen_GLOBAL_get_subprogram_scope_LIST(root),
                 LEVEL1, InitialInfo, NULL, (Generic)LocInfo);

 LocInfo->p->WriteLocalInfo((Generic)LocInfo);
/* LocInfo->m->write(LocInfo->dbport);*/
}

/**********************************************************************/
/* InitialInfo is written by each individual to determine the set of  */
/* local initial information that must be collected. It is called     */
/* by WalkInitialInformation and performs a specific operation on each*/
/* node of the Abstract Syntax Tree (AST)                             */
/**********************************************************************/
static int InitialInfo(AST_INDEX node, int level, Generic LocInfo)
{
 level = 1;
 switch (gen_get_node_type(node)) {
  case GEN_FUNCTION:
  case GEN_SUBROUTINE:
  case GEN_PROGRAM:
  case GEN_BLOCK_DATA:
  
  WalkInfoProc(node, (LInfo*)LocInfo);
  return WALK_CONTINUE; 
  break;
 
  default:
  return WALK_CONTINUE;
  }
}

/*********************************************************************/
/* This routine walks a procedure, subroutine, or function           */
/* It creates an entry and walks all the expressions looking for     */
/* specific local information                                        */
/*********************************************************************/
static void WalkInfoProc(AST_INDEX node, LInfo* LocInfo )
{
 AST_INDEX stmt_list, current;
 LocInfo->I = new ModRefNameTreeNode(SEQBLOCK, ft_NodeToNumber(LocInfo->ft, node), 0); 

 LocInfo_proc_sym_table(LocInfo, node);
 LocInfo->fd = new FortranDInfo();
 LocInfo->node = node;

 stmt_list = get_stmts_in_scope(node);
 
 assert(LocInfo->proc_sym_table != 0);
 
 WalkLocalInfo(stmt_list,(Generic)LocInfo);

}
/****************************************************************************/
/* called from WalkLocalInfo                                                */
/* walk_expression. Every node is invoked with the list of functions        */
/****************************************************************************/
static void WalkLocalInfo(AST_INDEX node, Generic LocalInfo)
{
 ((LInfo*)LocalInfo)->p->ComputeLocalInfo(node, LocalInfo);
}




