/* $Id: InitInfo.h,v 1.19 1997/03/11 14:29:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**************************************************************/
/* Author : Seema Hiranandani                                 */
/* Date   : 2/10/92                                           */
/* Class Definitions for computing initial interprocedural    */
/* information                                                */
/**************************************************************/
#ifndef InitInfo_h
#define InitInfo_h

#include <assert.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/fortTree/FortTree.i>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/support/tables/symtable.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <libs/support/file/FormattedFile.h>


/* C++ includes for mod/ref information */
#include <libs/frontEnd/fortTree/modrefnametree.h>

#if 0
#include <libs/ipAnalysis/ipInfo/module.h>
#endif

#include <libs/ipAnalysis/ipInfo/iptypes.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/frontEnd/fortTree/ilist.h>

#include <libs/fortD/misc/FortD.h>

/* extern declarations */

EXTERN(IPinfoTree*, loop_tree_info_for_entry, (Generic   LocInfo,
					       AST_INDEX node));

/************************************************************/
/* The base class for localInformation                      */
/************************************************************/
class InitialProblem {
  public:
    virtual void ComputeLocalInfo(AST_INDEX, Generic) 
    { // AST_INDEX node, Generic LInfo
       return;
    };

    virtual void WalkRoutine(AST_INDEX, int, Generic) 
    { // AST_INDEX node, int level, Generic LInfo
       return;
    };

    virtual void WriteLocalInfo(Generic) 
    { // Generic LocInfo
       return;
    };
};

/************************************************************/
/* the modref problem class derived from the base class     */
/************************************************************/
class ModRefProblem: public InitialProblem {
  private:
    ModRefType modreftype;
    ModRefNameTreeNode *I;

  public: 
    ModRefProblem();
    ~ModRefProblem();

    void  SetModRefType(ModRefType m)
    {
       modreftype = m;
    };

    void SetModRefNameTreeNode(ModRefNameTreeNode* T)
    {
       I = T;
    };

    ModRefNameTreeNode* GetModRefNameTreeNode() 
    {
       return I;
    };

    void ProblemInfo(ModRefType m, ModRefNameTreeNode* T)
    {
       modreftype = m;
       I = T;
    };

    ModRefType GetModRefType()
    {
       return modreftype;
    };


    void ModRef(AST_INDEX node, Generic LocalInfo);

    virtual void WalkRoutine(AST_INDEX, int, Generic) 
    { // AST_INDEX node, int level, Generic LocalInfo
       return;
    };

    virtual void ComputeLocalInfo(AST_INDEX node, Generic LocalInfo);
    virtual void WriteLocalInfo(Generic LocInfo);
};


/************************************************************/
/* the fortranD problem class derived from the modref class */
/************************************************************/
class FortranDProblem: public ModRefProblem {
  public:

#if 0
  void write(Generic LocInfo);
#endif
//  void read(Context     context, char *name);
  virtual void ComputeLocalInfo(AST_INDEX node, Generic LInfo);
  virtual void WalkRoutine(AST_INDEX node, int level, Generic LocInfo);
 
/* write the fortranD specific info once modref is written */    
  virtual void WriteLocalInfo(Generic LocInfo){
#if 0
   ModRefProblem::WriteLocalInfo(LocInfo);
   write(LocInfo);
#endif
   assert(0);
 };
};



/********************************************************************/
/* Alignment/Distribute problem class derived from the modref class */
/********************************************************************/
class AlignDistributeProblem: public ModRefProblem {

  public:
  virtual void ComputeLocalInfo(AST_INDEX node, Generic LInfo) {};
};
 
/*****************************************************************/
/* The structure that contains objects needed for all the local  */
/* information classes                                           */
/*****************************************************************/
class LInfo {
  public:
    InitialProblem *p;
    FortTree ft;
    SymDescriptor proc_sym_table;
    AST_INDEX node;
    ModRefNameTreeNode *I;
    struct AlignDistributeType *AD;
#if 0
    ModuleIPinfoList *m;
#endif
    IPinfoTree *ipt;
    FortranDInfo *fd;
    IPinfoTreeNode *tnode; 
    FormattedFile *dbport; 
    Context c;
 
    LInfo(FortTree fortt, Context contxt)
      {
        ft = fortt;
        c = contxt;
	ipt = 0;
#if 0
        m = new ModuleIPinfoList;  
#endif
      };

    LInfo(FortTree fortt, SymDescriptor d)
      {
        ft = fortt;
	ipt = 0;
#if 0
        m = new ModuleIPinfoList;
#endif
        proc_sym_table = d;
      };
};

#endif
