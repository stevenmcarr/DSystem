/* $Id: driver.h,v 1.12 1997/03/11 14:28:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef fort_d_driver_h
#define fort_d_driver_h

#include <string.h>

#include <libs/support/misc/general.h>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/fortD/codeGen/FortDInterface.h>

#include <libs/ipAnalysis/ipInfo/iptypes.h>

#include <libs/support/lists/IOSinglyLinkedList.h>

#include <libs/support/tables/HashTable.h>
#if 0
#include <libs/support/database/OBSOLETE/AsciiDbioPort.h>
#endif
#include <libs/support/strings/rn_string.h>
#include <libs/support/lists/SinglyLinkedList.h>

#include <libs/ipAnalysis/problems/fortD/ReachAnnot.h>
#include <libs/ipAnalysis/problems/fortD/ReachDFProblem.h>
#include <libs/ipAnalysis/problems/fortD/OverlapAnnot.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/main_dg.h>

#define FD_MAX_MODULE 20

//------------------------------------------------------------------------------
// forward declarations
//------------------------------------------------------------------------------

class FD_Composition_HT;

EXTERN (void, dc_compile_proc, (PedInfo ped, AST_INDEX node,
				int numprocs, Fd_opts *fd_opts, Generic di));
EXTERN (void, dc_compute_local, (AST_INDEX node, PedInfo ped));
EXTERN (void, dc_compute_local_decomp, (AST_INDEX node, FD_Composition_HT *f));

struct proc_in_module
{
  AST_INDEX  proc;
  Boolean    found;
  char*      name;
}; 

//------------------------------------------------------------------------------
// a hash table entry which contains the name of a subroutine or callsite,
// the ast, and type i.e. whether procedure, function or callsite
//------------------------------------------------------------------------------
class FD_ProcEntry
{
  public:
      // a pointer to its ast number
    AST_INDEX      ast;

      // function, procedure or callsite
    unsigned int   type;

      // index of the module array that contains info for this proc
    int mcount;

    SymDescriptor   proc_sym_table;

      // annotation that contains all the decomposition info for the proc
    FD_Reach_Annot* proc_annot; 

    unsigned int hash(unsigned int size) 
    { 
      return hash_string(nm, size); 
    };

    int compare(FD_ProcEntry *e1)
    { 
      return strcmp(this->name(), e1->name()); 
    };

    char* name(void) 
    { 
      return nm; 
    };

    void add_name(char *name) 
    { 
      nm = ssave(name); 
      return;
    };

    int mdcount(void) 
    { 
      return mcount; 
    };

      // store the read annotation
    void PutReadAnnotation(FD_Reach_Annot *annot, FD_Overlap_Annot *overlap_a)
    {
      proc_annot = annot;
      if(proc_annot->f == 0)
      proc_annot->f = new FortranDInfo();
      proc_annot->f->overlap_info = overlap_a->GetOverlapInf();
      return;
    };

    void PutReadAnnotation(FD_Reach_Annot *annot)
    {
      proc_annot = annot;
      return;
    };

    SymDescriptor symtable(void)
    {
      return proc_sym_table;
    };

    AST_INDEX get_ast(void)
    {
      return ast;
    };

  private:
    char*  nm;    // name of procedure, function or callsite with c_id appended
};

//----------------------------------------------------------------------
// contains a handle to the root, forttree and forttexttree for each
// module
//----------------------------------------------------------------------
class md_info
{
 public:
   FortTree     ft;
   FortTextTree ftt;
   AST_INDEX    root;
   Context      md;
   PedInfo      ped;

   PedInfo pedinfo(void) {return ped;};
   FortTree forttree(void) {return ft;};
   AST_INDEX ast_root(void) {return root;};
};

//-----------------------------------------------------------------------
// store the names and ast indices 
// of all the procedures and callsites in the composition
//-----------------------------------------------------------------------

class FD_Composition_HT : private HashTable 
{
  public:
    FD_Composition_HT(CallGraph *_callGraph);
    
    FD_ProcEntry *proc(void);

    void put_proc(FD_ProcEntry *p);

    void record(FortTree ft, FortTextTree ftt, AST_INDEX root, 
                Context md, PedInfo ped);

    FD_ProcEntry* add(char *name, AST_INDEX stmt, unsigned int type);

    FD_ProcEntry* GetEntry(char *name);

    PedInfo Ped(FD_ProcEntry *entry);

  public:
   CallGraph * const callGraph;

 private:
   int            count; 
   md_info        mod_info[FD_MAX_MODULE];
   FD_ProcEntry*  current_proc; 

   virtual uint HashFunct(const void *entry, const uint size);
   virtual int  EntryCompare(const void *e1, const void *e2);
   virtual void EntryCleanup(void *entry);
};


#endif fort_d_driver_h
