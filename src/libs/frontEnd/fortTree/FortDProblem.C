/* $Id: FortDProblem.C,v 1.10 1997/03/11 14:29:47 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-----------------------------------------------------------------------
// author   : Seema Hiranandani
// contents : Walk routines that walk the AST and collect fortran D related
//            information that includes reaching decompositions, overlap
//            estimation, numprocs specification, common block declaration
//            and common block variables reaching information
// date     : Since March 1992
//-----------------------------------------------------------------------

#include <libs/support/misc/general.h>

#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/InitInfo.h>

#include <libs/frontEnd/ast/ast_include_all.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/ast/treeutil.h>

#include <libs/fortD/misc/FortD.h>

#include <libs/support/tables/HashTable.h>
#include <libs/support/trees/NonUniformDegreeTree.h>

// forward declarations

static int WalkRDecomp(AST_INDEX node, int level, Generic LocInfo);
static int WalkRDecompExpr(AST_INDEX node, Generic LocInfo);
static void FDInvocation(AST_INDEX node, Generic LocInfo);

//-----------------------------------------------------------------------
// this function is a wrapper function for ReachingDecomp called from    
// walk_statements                                                       
//-----------------------------------------------------------------------
static int WalkRDecomp(AST_INDEX node, int level, Generic LocInfo)
{
   ((LInfo*)LocInfo)->p->WalkRoutine(node, level, LocInfo);
   return (WALK_CONTINUE);
}

//-----------------------------------------------------------------------
// this function is called from WalkRoutine, It is a wrapper for walking
// expressions 
//-----------------------------------------------------------------------
static int WalkRDecompExpr(AST_INDEX node, Generic LocInfo)
{ 
   int level = 0;
   ((LInfo*)LocInfo)->p->WalkRoutine(node, level, LocInfo);
   return (WALK_CONTINUE);
}

//-----------------------------------------------------------------------
// This function walks the entire expression and collects information       
// for FortranD                                                             
//-----------------------------------------------------------------------
void FortranDProblem::WalkRoutine(AST_INDEX node, int level, Generic LocInfo)
{
   NODE_TYPE       node_type = gen_get_node_type (node);
   AST_INDEX       elt;
   int             i,n;
   level = 1;
   switch (node_type) 
   {
      case GEN_COMMENT:
        if (((LInfo*)LocInfo)->fd->FortranD_dir(node))
	  switch(((LInfo*)LocInfo)->fd->comment_type)
	  {
	     case DECOMPOSITION:
	     case ALIGN:
	     case DISTRIBUTE:
	       ((LInfo*)LocInfo)->fd->StoreFortranD(node, ((LInfo*)LocInfo)->node,
				      ((LInfo*)LocInfo)->proc_sym_table,false);
	       break;

	     default:
	       break;
	  }
        break;

      case GEN_CALL:  
      case GEN_INVOCATION:
        /* accumulate a list of all invocations */
        FDInvocation(node, LocInfo);
        break;

      case GEN_PARAMETER:
        fd_store_param(node, ((LInfo*)LocInfo)->fd);
        break;

      case GEN_ASSIGNMENT:
        if (gen_get_node_type(((LInfo*)LocInfo)->node) == GEN_PROGRAM)
	   ((LInfo*)LocInfo)->fd->ComputeOverlapProgram(node);
        walk_expression(gen_ASSIGNMENT_get_rvalue(node), WalkRDecompExpr, NULL,
		      LocInfo);
        walk_expression(gen_ASSIGNMENT_get_lvalue(node), WalkRDecompExpr, NULL, 
		      LocInfo);
        break;

        case GEN_COMMON:
          ((LInfo*)LocInfo)->fd->ParseAndStoreCommon(node,
		    gen_get_text( get_name_in_entry( ((LInfo*)LocInfo)->node )),
					    ((LInfo*)LocInfo)->proc_sym_table );
        default:
          break;
   }
}

//-----------------------------------------------------------------
// this function initiates the FortranD problem           
// the constructor for FortranDInfo opens the fort text tree
//-----------------------------------------------------------------
void
FortranDProblem::ComputeLocalInfo(AST_INDEX stmt_list, Generic LocalInfo)
{
   AST_INDEX node, current;

   ModRefProblem::ComputeLocalInfo(stmt_list, LocalInfo);

   node = ((LInfo*)LocalInfo)->node;

   ((LInfo*)LocalInfo)->fd = 0;
   ((LInfo*)LocalInfo)->fd = 
     new FortranDInfo( 
		      ((LInfo*)LocalInfo)->ft, ((LInfo*)LocalInfo)->c);
   ((LInfo*)LocalInfo)->tnode->fd =  (HashTable*)(((LInfo*)LocalInfo)->fd);
   
   walk_statements(node, LEVEL1, WalkRDecomp, NULL, LocalInfo);

   switch (gen_get_node_type(node))
   {
      case GEN_FUNCTION:
      case GEN_SUBROUTINE:
      case GEN_BLOCK_DATA:
        ((LInfo*)LocalInfo)->fd->StoreOverlap(node,((LInfo*)LocalInfo)->fd->overlap_info);
        break;
   }
}


/*------------------------------------------------------------------
  FDInvocation()

  Process a callsite. Add information about the decompositions 
  that reach each procedure parameter
  */

static void FDInvocation(AST_INDEX node, Generic LocInfo)
{
   Boolean done = false;
   AST_INDEX invocation;
   char *name, *c_name;
#if 0
   ModuleIPinfoListEntry *m_entry;
#endif
   int callsite_id;
   CallSite *c_entry;
   NODE_TYPE node_type;

   name = gen_get_text((get_name_in_entry(((LInfo*)LocInfo)->node)));

   // if it's a call, get the invocation
   if ((node_type = gen_get_node_type(node)) == GEN_CALL)
     invocation = gen_CALL_get_invocation(node);
   else
     // else it's an invocation
     invocation = node; 
   c_name = gen_get_text(gen_INVOCATION_get_name(invocation));

#if 0
   /* get the ModuleIPInfoListEntry for the subroutine */

   m_entry = ((LInfo*)LocInfo)->m->First();
   while (!done) 
   {
      if (!strcmp(name, m_entry->info->name))
      {
	 done = true;
	 // cout<<form("procedure name = %s \n", m_entry->info->name);
	 
	 for(ParameterListEntry *k = m_entry->info->plist->First(); k != 0; 
	     k = m_entry->info->plist->Next())
	 {
	    // cout<<form("parameter name = %s \n", k->name());
	 }
      } else
	m_entry = ((LInfo*)LocInfo)->m->Next();
   }
#endif
   callsite_id = ft_NodeToNumber(((LInfo*)LocInfo)->ft, invocation);

   /* get the CallSiteEntry structure  */
   for( NonUniformDegreeTreeIterator 
       it(((LInfo*)LocInfo)->ipt->tree, PREORDER);
       it.Current() != 0; ++it) 
   {
      CallSitesIterator callsites(((IPinfoTreeNode*)it.Current())->calls);
      for(CallSite *c; c = callsites.Current(); callsites++) 
      {
	 if (callsite_id == c->Id()) 
	 {
	    ((LInfo*)LocInfo)->fd->StoreReachDecomp(c);
	    break;
	 }
      }
   }
}
