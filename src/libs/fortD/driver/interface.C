/* $Id: interface.C,v 1.7 1997/03/27 20:32:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author   : Seema Hiranandani
//
// content  : The procedures in this file drive the code generation
//            phase of the fortran D compiler
// date     : August 1992
//-------------------------------------------------------------------------

#include <iostream.h>

#include <libs/frontEnd/fortTree/ft.h>
#include <libs/support/database/context.h>
#include <libs/fortD/driver/driver.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/include/walk.h>

#include <libs/fortD/misc/fd_code.h>
#include <libs/fortD/codeGen/FortDInterface.h>

#include <libs/support/tables/cNameValueTable.h>


struct proc_list {
  AST_INDEX node[100];
  int count;
};

extern char* FORTD_REACH_ANNOT;
extern char* FORTD_COMMON_BLOCK_ANNOT;
extern char* FORTD_OVERLAP_ANNOT;

//------------------global declarations ------------------
EXTERN(Generic, dc_irreg_init, (FortTree ft, FortTextTree ftt,
				Fd_opts *fd_opts));

// extern void dc_compile_proc_p
//                             (PedInfo ped, AST_INDEX pnode, FortDInterface* fi);
//---------------forward declarations---------------------
// int procs_and_functions(AST_INDEX stmt, int level, Generic ht);

//------------------------------------------------------------------
// return the call graph
//------------------------------------------------------------------
void* FortDInterfaceCompSetUp(Composition *program)
{
  return ATTACH_ATTRIBUTE(program, CallGraph);
}
//------------------------------------------------------------------
// store a list of procedures and functions in this module
//------------------------------------------------------------------
int procs_and_functions(AST_INDEX node, int level, Generic p)
{
  switch(gen_get_node_type(node))
  {
  case GEN_FUNCTION:
  case GEN_SUBROUTINE:
  case GEN_PROGRAM:
    ((struct proc_list*)p)->node[((struct proc_list*)p)->count++] = node;
  }
  return WALK_CONTINUE;
}

//------------------------------------------------------------------
// 1. traverse the AST, perform local reaching decomposition
// 2. walk the tree, storing sp pointers at each reference
//------------------------------------------------------------------
static void ModuleSetUp(AST_INDEX root,PedInfo ped, FortTree ft, Context c, CallGraph* cg)
{
  AST_INDEX pnode;
  Generic dummy;
  CallGraphNode *n;
  struct proc_list p;
  FortDInterface *fd_in;

  FD_Composition_HT *fortd_ht = new FD_Composition_HT(cg);

//------------------------------
// initialize p structure
  p.count = 0;

//----------------------------------------------  
// at this point the call graph has been built


//---------------------------------------------------
// get the list of procedures in the the module

  walk_statements(root, LEVEL1, procs_and_functions, NULL, (Generic)&p);

//------------------------------------------------------------
// for each procedure get the reach and overlap annotation 

 fortd_ht->record(ft, 0, root, 0, ped);
   
 for( int i = 0; i < p.count; i++) {
   n = cg->LookupNode(gen_get_text(get_name_in_entry(p.node[i])));

   if (n == 0)
     cout << "Unable to find node "
          << gen_get_text((get_name_in_entry(p.node[i])))
          << " in call graph \n";
    
  FD_Reach_Annot* reach_annot = 
     (FD_Reach_Annot*)n->GetAnnotation(FORTD_REACH_ANNOT, true);
  reach_annot->MapReachNodeAnnot(n);
  
  FD_Overlap_Annot *overlap_annot = 
    (FD_Overlap_Annot*)n->GetAnnotation(FORTD_OVERLAP_ANNOT, true);

  fortd_ht->add(n->procName, p.node[i], gen_get_node_type(p.node[i]));
  
  FD_ProcEntry *entry = fortd_ht->GetEntry(n->procName);
  entry->PutReadAnnotation(reach_annot, overlap_annot);
  fortd_ht->put_proc(entry);

//---------------------------------------------------------
// store the ft and context in the Fortran D info structure

  fortd_ht->proc()->proc_annot->f->ContextFt(c, ft);

//----------------------------------------------- 
// perform local decomposition analysis
 
  dc_compute_local_decomp(entry->ast, fortd_ht);
  }
//--------------------------------------------------
// START THE D INTERFACE ON EACH PROCEDURE
//---------------------------------------------------
 
  // fd_in = new FortDInterface(ped,root,c,cg,ft);
  // dc_compile_proc_p(ped, root, fd_in);

// Initialize irregular part
//  di = dc_irreg_init(ft, 0, fd_opts);  


}


//------------------------------------------------------------------
// Test the fortran d interface 
//------------------------------------------------------------------
void FortDInterfaceTest(CallGraph *cg, Composition *comp)
{
 char *name;
 AST_INDEX root;
 FortTree ft;
 FortTextTree ftt;
 PedInfo ped;
 char *nme;
 FortDInterface *fd_in; 
 cNameValueTable analyses =
 NameValueTableAlloc(8, (NameCompareCallback) strcmp,
		     (NameHashFunctCallback) hash_string);

//-----------------------------------------------------
// record the fortran d annotations in the hash table  
 
 Generic dummy;

 NameValueTableAddPair(analyses, (Generic)FORTD_REACH_ANNOT, 0, &dummy);
 NameValueTableAddPair(analyses, (Generic)FORTD_COMMON_BLOCK_ANNOT, 0, &dummy);
 NameValueTableAddPair(analyses, (Generic)FORTD_OVERLAP_ANNOT, 0, &dummy);
  
 CompModulesIterator modules(comp);
 FortranModule *module;
 for (; module = (FortranModule *) modules.Current(); ++modules) {
   FortTreeModAttr *ftAttr = ATTACH_ATTRIBUTE(module, FortTreeModAttr);
   FortTextTreeModAttr *fttAttr = ATTACH_ATTRIBUTE(module, FortTextTreeModAttr);
   
   ft   = ftAttr->ft;
   ftt  = fttAttr->ftt;
   
   root = ft_Root(ft);
   
   Boolean  has_errors = false;
  
   ped  = (PedInfo) pedInitialize(0, ftt, ft, module, CONTEXT_NULL, CONTEXT_NULL, has_errors,false);
   
   // fd_in = new FortDInterface(ped,root,contxt,cg, ft); 
   // dc_compile_proc_p(ped, root, fd_in);
//  ModuleSetUp(root, ped, ft, contxt, cg);
   
   /* do this prior to closing ftt and ft */
   pedFinalize(ped);
   
   module->DetachAttribute(ftAttr);
   module->DetachAttribute(fttAttr);
 }
}


