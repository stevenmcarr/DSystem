/* $Id: driver.C,v 1.23 1997/03/11 14:28:21 carr Exp $ */
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


#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/NeedProvCompAttr.h>
#include <libs/fileAttrMgmt/composition/ProcModuleMap.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>


#include <libs/frontEnd/ast/ast.h>

#include <libs/frontEnd/fortTree/fortsym.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTree/FortTree.h>

#include <libs/frontEnd/include/walk.h>

#include <libs/frontEnd/fortTree/ft.h>

#include <libs/fortD/driver/driver.h>
#include <libs/fortD/misc/fd_code.h>
#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/codeGen/FortDInterface.h>

#include <libs/support/tables/cNameValueTable.h>
#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

#include <libs/fortD/performance/instr/InstrumentSPMD.h>
#include <libs/fortD/performance/staticInfo/SDDF_Instrumentation.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/main_dg.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>


extern char* FORTD_REACH_ANNOT;
extern char* FORTD_COMMON_BLOCK_ANNOT;
extern char* FORTD_OVERLAP_ANNOT;

/*------------------ GLOBAL DECLARATIONS --------------------*/

EXTERN(Generic, dc_irreg_init,      (FortTree     ft,
				     FortTextTree ftt,
				     Fd_opts      *fd_opts,
				     FortranDInfo *fd));

extern void FortDInterfaceTest(CallGraph* cg, Composition *comp);

//-------------------------
// forward definitions

STATIC(int, procs_and_calls, (AST_INDEX stmt, int level, Generic ht));
STATIC(void, CompileFortDProcedure, (CallGraphNode *v, FD_Composition_HT *fortd_ht,
				     Composition *comp, Fd_opts *fd_opts, CallGraph*));
STATIC(void, CleanUpFortDProcedure, (CallGraphNode *n));

EXTERN (void, dc_compile_proc, (PedInfo ped, AST_INDEX node,
				int numprocs, Fd_opts *fd_opts, Generic di));
//------------------------------------------------------------------
// 1. get the call graph 
// 2. set up the flow as bottom up 
// 3. get the list of modules and parameters 
// 4. store the ast addresses of each procedure in a hash table
// 5. read in the interprocedural annotations containing reaching
//    decompositions 
// 6. for each procedure call the fortrand compiler 
// 7. generate code and if there are any loop independent dependences
// 8. from outside the loop, store it as an annotation to the edge(s)
//------------------------------------------------------------------
void fortd_compiler(cNameValueTable analyses, Fd_opts *fd_opts)
{
  
  Composition program;
  Boolean has_errors;
  CallGraphNode *n; 
 
  SPMDInstrumentation *instr = new SPMDInstrumentation(fd_opts);
  // Should be a class, but not right now. MAA 94/6/17
  SD_InitialSetup(instr);
  
  //-----------------------------------------------------------------
  // register the annotations in the hash table
  // obtain the program context
  // open the composition
  // read the local information for each procedure
  // construct the call graph
  // demand annotations; this starts up the data flow solver
  // generate code, after the data flow problems have been solved
  //-----------------------------------------------------------------
  Generic dummy;
  
  NameValueTableAddPair(analyses, (Generic)FORTD_REACH_ANNOT, 0, &dummy);
  NameValueTableAddPair(analyses, (Generic)FORTD_COMMON_BLOCK_ANNOT, 0, &dummy);
  NameValueTableAddPair(analyses, (Generic)FORTD_OVERLAP_ANNOT, 0, &dummy);
  
  if (program.Open(fd_opts->pgm) != 0) {
    fprintf(stderr, "Unable to open %s \n", fd_opts->pgm);
    exit(-1);
  }
  
  if (program.IsCompleteAndConsistent() == false) {
    fprintf(stderr, "Fatal error: composition %s has errors. Exiting ...\n", 
	    fd_opts->pgm);
    exit(-1);
  }
  
  CallGraph *callGraph = ATTACH_ATTRIBUTE(&program, CallGraph); 
  FD_Composition_HT *fortd_ht = new FD_Composition_HT(callGraph);

  
  //---------------------------
  // if flag set, invoke fortran d interface
  //---------------------------
  
  if (fd_opts->flags[Do_interface])
    {
      cout<<form("Invoking the D Editor interface \n");
      FortDInterfaceTest(callGraph, &program);
    }
  
  //-------------------------------------
  // else invoke the fortran d compiler
  //-------------------------------------
  
  else  
    {  
      cout<<form("Invoking the Fortran D code generator \n");
      
      // 2/9/94 RvH: Determinism helps debugging
      //for(CallGraphNodeIterator x(cg2, Unordered); n = x.Current(); x++)
      for(CallGraphNodeIterator x(callGraph, PostOrder); n = x.Current(); x++) {
	switch(n->type) {
	case CGNT_Function:
	case CGNT_Program:
	case CGNT_Subroutine:
	  CompileFortDProcedure(n, fortd_ht, &program, fd_opts, callGraph);
	  break;
	case CGNT_Entry:
	case CGNT_Exit:
	case CGNT_Start:
	  // ignore nodes in the CallGraph that do not represent real 
	  // procedures (that have been inserted merely to simplify dataflow 
	  // analysis) -- 13 Sept 1994 John Mellor-Crummey
	  break;
	case CGNT_ProcParCaller:
	  // there is no support for handling procedure parameters in the
	  // Fortran D interprocedural analysis code
	  // -- 13 Sept 1994 John Mellor-Crummey
	  assert(0); 
	case CGNT_BlockData:
	  // I do not think there is any support for block data
	  // -- 13 Sept 1994 John Mellor-Crummey
	  assert(0); 
	case CGNT_PlaceHolder:
	  // should NEVER see this -- 13 Sept 1994 JMC
	  assert(0); 
	}
      }
    }
  
  program.DetachAttribute(callGraph);
  program.Close();
  
  // the fortran d compiler returns the edge annotations for each of
  // the call sites in the procedure
  
  // the annotations contain information about sends/receive that must be
  // placed at the call site
  
  // Perform a top down pass to write out all the send/recv information at
  // the call sites

  // Clean up the Instrumentation stuff -- MAA 94/6/17
  SD_FinalCleanupAndOutput();  
  
  delete instr;				// delete SPMDInstrumentation object
}


//------------------------------------------------------------------
// For each procedure save its name, type, ast address and symbol
// table in a hash table. This information will be used later on
// when local reaching decomposition is performed and during
// code generation 
//------------------------------------------------------------------
static int procs_and_calls(AST_INDEX node, int level, Generic p)
{
  char *name;
  struct proc_in_module *pt;
  
  switch(gen_get_node_type(node))
  {
  case GEN_FUNCTION:
  case GEN_SUBROUTINE:
  case GEN_PROGRAM:
    
    name = gen_get_text(get_name_in_entry(node));
    pt   = (struct proc_in_module *)p;
    
    if(strcmp(name, pt->name) == 0)
    {
      pt->proc = node;
      pt->found = true;
      return WALK_ABORT;
    }
    
    break;
  }
  return WALK_CONTINUE;
}

//------------------------------------------------------------------
// FD_Composition_HT public member functions
//------------------------------------------------------------------

FD_Composition_HT::FD_Composition_HT(CallGraph *_callGraph) 
  : HashTable(), count(0), callGraph(_callGraph)
{
  HashTable::Create(sizeof(FD_ProcEntry), 8);

  for (int i = 0; i < FD_MAX_MODULE; ++i)
  {
     mod_info[i].ft   = NULL;
     mod_info[i].ftt  = 0;
     mod_info[i].root = AST_NIL;
     mod_info[i].md   = 0;
     mod_info[i].ped  = NULL;
  }
}
   
FD_ProcEntry* FD_Composition_HT::proc()
{
  return current_proc;
}

void FD_Composition_HT::put_proc(FD_ProcEntry *p)
{
  current_proc = p;

  return;
}

FD_ProcEntry* FD_Composition_HT::GetEntry(char *name)
{
  FD_ProcEntry e, *found = NULL;

  e.add_name(name);
  found = (FD_ProcEntry*)HashTable::QueryEntry(&e);

  return found;
}

PedInfo FD_Composition_HT::Ped(FD_ProcEntry *entry)
{
  int     mcount = entry->mdcount();
  md_info m_info = mod_info[mcount];
  PedInfo ped    = m_info.pedinfo();

  return ped;
}

//------------------------------------------------------------------
// FD_ProcEntry contains the following information for a procedure
// 0) procedure name
// 1) the symbol table for the procedure
// 2) type i.e. procedure, function, program
// 3) ast node number of the procedure
//  An entry for each procedure is stored in FD_Composition_HT,
//  Fortran D Composition Hash Table
//------------------------------------------------------------------
FD_ProcEntry* 
FD_Composition_HT::add(char *name, AST_INDEX stmt, unsigned int tp)
{
  FD_ProcEntry  e, *found;
  SymDescriptor proc_sym_table;
  
  e.add_name(name);
  found = (FD_ProcEntry*)HashTable::QueryEntry(&e);
  
  if(found == 0)
  {
    proc_sym_table =  fst_GetTable(mod_info[count-1].ft->td, name);
    e.ast  = stmt;
    e.type = tp;
    e.mcount = count-1;
    e.proc_sym_table = proc_sym_table;
    HashTable::AddEntry(&e);
    found = (FD_ProcEntry*)HashTable::QueryEntry(&e);
  }
  return(found);
}

//------------------------------------------------------------------
// records  information on a module. Includes the AST tree, the
// database id for the module, and a handle to ped
//------------------------------------------------------------------
void  FD_Composition_HT::record
(FortTree ft, FortTextTree ftt, AST_INDEX root, Context md, PedInfo ped)
     
{
  mod_info[count].ft   = ft;
  mod_info[count].ftt  = ftt;
  mod_info[count].root = root;
  mod_info[count].md   = md;
  mod_info[count].ped  = ped;
  ++count;
}

//------------------------------------------------------------------
// FD_Composition_HT private member functions
//------------------------------------------------------------------

uint FD_Composition_HT::HashFunct(const void *entry, const uint size)
{
  return ((FD_ProcEntry*)entry)->hash(size);
}

int FD_Composition_HT::EntryCompare(const void *e1, const void *e2)
{
  return ((FD_ProcEntry*)e1)->compare((FD_ProcEntry*)e2);
}

void FD_Composition_HT::EntryCleanup(void *entry)
{
  return;
}


//------------------------------------------------------------------
// 1. traverse the AST, perform local reaching decomposition
// 2. walk the tree, storing sp pointers at each reference
//------------------------------------------------------------------
static void CompileFortDProcedure(CallGraphNode *n,
				  FD_Composition_HT *fortd_ht,
				  Composition *program, Fd_opts *fd_opts, CallGraph *cg)
{
  PedInfo ped;
  AST_INDEX pnode;
  FortTree ft;
  FortTextTree ftt;
  AST_INDEX root;
  struct proc_in_module p;
  Boolean  has_errors;
  FD_Reach_Annot *reach_annot;
  Generic di;                       // Data for irregular phase

  cout << "Compiling " << n->procName  << " ..." << endl ;
  
  //-------------------------------------------------------------------
  // get the annotation, it contains all the decomposition information
  
  reach_annot = (FD_Reach_Annot*)n->GetAnnotation(FORTD_REACH_ANNOT, true);
  reach_annot->MapReachNodeAnnot(n);
  //-------------------------------------------------------------------
  // get the annotation, it contains all the overlap information
  
  FD_Overlap_Annot *overlap_annot = 
    (FD_Overlap_Annot*)n->GetAnnotation(FORTD_OVERLAP_ANNOT, true);
  
  //-------------------------------------------------------------------
  // build a context for the module containing this entry point 
  
  
  NeedProvCompAttr *npc = ATTACH_ATTRIBUTE(program, NeedProvCompAttr);
  ProcModuleMapEntry *pme = npc->provides->QueryEntry(n->procName);
  
  const char *mod_name = pme->moduleName;
  program->DetachAttribute(npc);

  FortranModule *module = (FortranModule *) program->GetModule(pme->moduleName);

  FortTreeModAttr *ftAttr = ATTACH_ATTRIBUTE(module, FortTreeModAttr);
  module->SaveAttribute(ftAttr);
  FortTextTreeModAttr *fttAttr = ATTACH_ATTRIBUTE(module, FortTextTreeModAttr);
  module->SaveAttribute(fttAttr);

  ft   = ftAttr->ft;
  ftt  = fttAttr->ftt;

  root = ft_Root(ft);
  

  //--------------------------------------------------
  // walk the module looking for the subroutine
  // if and when it's found, return the ast number
  
  p.name = ssave(n->procName);
  p.found = false;
  walk_statements(root, LEVEL1, procs_and_calls, NULL, (Generic)(&p));   
  
  assert(p.found);

  // Build the annotations (lineno, etc) for Pablo SDDF info. 94/6/16 MAA.
  // Must do this *before* stripping the tree to get correct line numbers.
  SD_Build_Tree_Annot(module,n->procName,p.proc,ftt,ft);
  
  {
   // Patch to strip out all but the procedure to be compiled, 
   // instead of analyzing the entire input file
   // JMC 9/21/94

	AST_INDEX stmtlist, stmt, nextstmt;
	assert(gen_get_node_type(root) == GEN_GLOBAL);
	stmtlist = gen_get_stmt_list(root);

	for(stmt = list_first(stmtlist); stmt != AST_NIL; stmt = nextstmt) {
		nextstmt = list_next(stmt);
		if (stmt != p.proc) { 
			list_remove_node(stmt);
			tree_free(stmt);
		}
	}
  }


  // Initialize irregular part
  di = dc_irreg_init(ft, ftt, fd_opts, reach_annot->f);

  // ----- presumably, but this should be checked -----
  
  has_errors = false;
  ped  = (PedInfo) pedInitialize(0, ftt, ft, program, CONTEXT_NULL, CONTEXT_NULL, has_errors,false);   
  
  fortd_ht->record(ft, ftt, root, program, ped);
  fortd_ht->add(p.name, p.proc, gen_get_node_type(p.proc));   
  
  //---------------------------------------------------
  // get the node that corresponds to the procedure
  
  FD_ProcEntry *entry = fortd_ht->GetEntry(n->procName);
  pnode =  entry->ast;
  
  entry->PutReadAnnotation(reach_annot, overlap_annot);
  
  fortd_ht->put_proc(entry);

//---------------------------------------------------------
// store the ft and context in the Fortran D info structure

  fortd_ht->proc()->proc_annot->f->ContextFt(program, ft);  

  //---------------------------------------------
  // perform local decomposition analysis
  // store the sp in the side array

  dc_compute_local_decomp(pnode, fortd_ht);
  ped = fortd_ht->Ped(fortd_ht->proc());
  
  //--------------------------------------------------
  // START THE FORTRAN D COMPILER ON EACH PROCEDURE
  //---------------------------------------------------

  // New SDDF generation stuff added by Mark Anderson (MAA)
  SD_InitLocalAndGatherInfo(ftt,ft);
 

  FortDNumProcsAnnot *nProcsAnnot = (FortDNumProcsAnnot *) 
    fortd_ht->callGraph->GetAnnotation(FORTD_NUM_PROCS_ANNOT, true);

  assert(nProcsAnnot);


   dc_compile_proc(ped, pnode, nProcsAnnot->numberProcs, fd_opts, di);
  
  // Do any cleanup needed after generating SDDF records (MAA)
  SD_CleanupLocal();

  /* do this prior to closing ftt and ft */
  pedFinalize(ped);
  
  module->DetachAttribute(fttAttr, CACHE_FLUSH_IMMEDIATE);
  module->DetachAttribute(ftAttr, CACHE_FLUSH_IMMEDIATE);

}

//------------------------------------------------------------------
//------------------------------------------------------------------
static void CleanUpFortDProcedure(CallGraphNode *n)
{
}
