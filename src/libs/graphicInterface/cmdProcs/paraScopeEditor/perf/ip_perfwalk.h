/* $Id: ip_perfwalk.h,v 1.1 1997/03/11 14:32:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/* $Id: ip_perfwalk.h,v 1.1 1997/03/11 14:32:11 carr Exp $
*/

/*
 * Declarations private to the IP local phase of the performance
 * estimator. This header file should not be included elsewhere. 
 */

/*
 * Revision History:
 *
 * $Log: ip_perfwalk.h,v $
 * Revision 1.1  1997/03/11 14:32:11  carr
 * newly checked in as revision 1.1
 *
 * Revision 1.11  93/12/17  14:55:44  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.10  93/08/11  16:22:31  mcintosh
 * Revise comments; change scheme for mapping AST nodes to partial
 * performance estimates.
 * 
 * Revision 1.9  93/06/11  13:47:11  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.8  93/05/25  15:59:58  curetonk
 * *** empty log message ***
 * 
 * Revision 1.7  93/04/28  10:46:50  mcintosh
 * Various changes, including: a loop stack data structure to support
 * better guesses for loop upper bounds based on array acess patterns,
 * moved some code here from ip_perfwalk.C to support ip_perfutil.C.
 * 
 * Revision 1.6  93/03/17  13:18:08  mcintosh
 * update to reflect new names/locations of IP perf
 * est header files. 
 * 
 * Revision 1.5  92/12/14  21:56:05  mcintosh
 * Improve comments (no changes to functionality).
 * 
 * 
 */

#ifndef ip_perfwalk_h
#define ip_perfwalk_h

/* ---- Include files ---- */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef rn_string_h
#include <libs/support/strings/rn_string.h>
#endif

#ifndef mem_h
#include <libs/support/memMgmt/mem.h>
#endif

#ifndef fort_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
#endif

#ifndef fortsym_h
#include <libs/frontEnd/fortTree/fortsym.h>
#endif

#ifndef walk_h
#include <libs/frontEnd/include/walk.h>
#endif

#ifndef newdatabase_h
#include <libs/support/database/newdatabase.h>
#endif

#ifndef FortTree_h
#include <libs/frontEnd/fortTree/FortTree.h>
#endif

#ifndef dg_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dg.h>
#endif

#ifndef cd_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cd.h>
#endif

#ifndef private_cd_h
#include <libs/moduleAnalysis/dependence/controlDependence/private_cd.h>
#endif

#ifndef builtins_h
#include <libs/frontEnd/ast/builtins.h>
#endif

#ifndef ip_perfdata_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perfdata.h>
#endif

#ifndef ip_perf_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perf.h>
#endif

#ifndef PerfEstExpr_h
#include <libs/ipAnalysis/problems/perfEst/PerfEstExpr.h>
#endif

#ifndef jumptrans_h
#include <libs/moduleAnalysis/cfgValNum/jumptrans.h>
#endif

#ifndef cdgwalklist_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/cdgwalklist.h>
#endif

#ifndef pt_util_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#endif

/* ---- Misc. constants and macros ---- */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define NESTLEVEL0	0
#define PERF_MEM_ID "perf_local_phase"    /* given to "get_mem" */

/* ---- C++ classes ---- */

/*
 * Create a subclass of CDGWalkList which customizes each list entry for
 * our purposes. For each node in the control dependence graph, we want to
 * record a cost expression. Then when its control-dependence parent is
 * visited, we want to build the parents's expression based on the
 * expressions of its children. We use the PerfWalkList to store the child
 * expressions so the parent can get at them.
 */

class PerfWalkListEntry : public CDGWalkListEntry {
 public:
  PerfEstExprHandle expr;
  PerfWalkListEntry(AST_INDEX new_ai, cdNode *new_cdn, PerfEstExprHandle e);
  virtual ~PerfWalkListEntry();
};

class PerfWalkList : public CDGWalkList {
  
 public:

  /* Override methods from parent class
   */
  void append_entry(PerfWalkListEntry *e);

  /* This method is just "syntactic sugar"
   */
  void create_and_append_item(AST_INDEX ai, cdNode *cdn,
			      PerfEstExprHandle e);
};

class PerfWalkListIterator : private CDGWalkListIterator {
 public:
  PerfWalkListIterator(PerfWalkList *l, cdNode *src,
		       CDGWalkList_control cntrl);
  PerfWalkListEntry *next_entry();
  PerfWalkListEntry *current();
};

typedef struct PerfLoopGuess_tag {
  int dims;
  int intg;
  char *varg;
  Boolean valid;
  Boolean assign;
  Boolean controldep;
} PerfLoopGuess;

/* An entry in the loop stack. See below for more info.
*/

class PerfLoopStackEntry : private SinglyLinkedListEntry {
 public:
  PerfLoopStackEntry(AST_INDEX node,
		     char *indvar,
		     Boolean triangular,
		     PerfLoopGuess guess);
  ~PerfLoopStackEntry();

  AST_INDEX node;	/* AST index of loop */
  char *indvar;		/* name of induction variable */
  Boolean triangular;	/* TRUE if this is a triangular loop */
  PerfLoopGuess guess;	/* current guesses about loop */
};

/*
 * One of the heuristics used by the performance estimator to guess loop
 * upper bounds is to look at the dimensions of the arrays which the loop
 * induction variable accessed. For example, if the variable is used to
 * access an array of size 10, then this is a hint that the loop probably
 * runs 10 iterations or less.
 * 
 * To implement this, when we see an array reference, we need to have all the
 * information about the loops which enclose it. We take care of this by
 * maintaining a stack. As we descend into loops (the "preorder" portion
 * of the CDG walk) we push loops onto the stack; as we ascend back out of
 * loops (the "postorder" portion of the walk) we pop them off the stack.
 */

class PerfLoopStack : public SinglyLinkedList {
 public:
  PerfLoopStack();
  ~PerfLoopStack();

  /*
   * Override various existing methods from the parent class so we can use
   * them without getting argument-type warnings.
   */
   void push_entry(PerfLoopStackEntry *e);
   PerfLoopStackEntry *pop_entry();
   PerfLoopStackEntry *first_entry();
};

class PerfLoopStackIterator : public SinglyLinkedListIterator {
 public:
  PerfLoopStackIterator(PerfLoopStack *l) 
     : SinglyLinkedListIterator((SinglyLinkedList *) l) {};
  PerfLoopStackEntry *Current() {
    return (PerfLoopStackEntry *) SinglyLinkedListIterator::Current();
  };
};

/*
 * It is very useful for debugging purposes to be able to map an AST index
 * to a performance estimate. Since efficiency is not a major concern, we
 * implement this mapping using a linked list. Each list entry is a record
 * with an AST index and a pointer to a performance estimate.
 */

class ASTtoPerfMapListEntry : public SinglyLinkedListEntry {
 private:
  AST_INDEX i; /* a subtree */
  double v;    /* estimated time to execute that subtree */
 public:  
  ASTtoPerfMapListEntry(AST_INDEX ni, double nv) { i = ni; v = nv; };
  ~ASTtoPerfMapListEntry() { };
  AST_INDEX ai() { return i; };
  double pe() { return v; };
};

class ASTtoPerfMapList : public SinglyLinkedList {

 public:
   
  // The following are provided simply to allow the parent methods
  // to be used without getting type errors. 
  void Append(ASTtoPerfMapListEntry *e) {
    SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
  };
};

class ASTtoPerfMapListIterator : public SinglyLinkedListIterator {
 public:
  ASTtoPerfMapListIterator(ASTtoPerfMapList *l) :
    SinglyLinkedListIterator((SinglyLinkedList *) l) { };
  
  ASTtoPerfMapListEntry *Current() {
    return (ASTtoPerfMapListEntry *) SinglyLinkedListIterator::Current();
  };
};

/*
 * The following structure holds the information we will need while
 * walking the statements of the AST via the control dependence graph. A
 * single instance of this structure is used to carry information around
 * during the walk.
 * 
 * The field 'smap' is for debugging. It can be viewed as a linked list of
 * tuples (X,Y) where X is an AST_INDEX and Y is the partial performance
 * estimate for that subtree.
 */

typedef struct Treewalk_tag  {
  Perf_data *pdata;		/* performance data for this architecture */
  PerfEstExpr *t;		/* performance estimate expr being built */
  PerfWalkList *elist;		/* list of recently constructed estimates */
  FortTree ft;			/* FortTree for getting node numbers */
  ASTtoPerfMapList *smap;	/* see above */
  JumpTransInfo jumptrans;	/* handle from jump_trans_ ... routines */
  SymDescriptor sd;		/* symbol table for function */
  PerfLoopStack *loopstack;	/* enclosing loop info for walk */
} Treewalk;


extern double pe_debug_map_stmt_to_est(ASTtoPerfMapList *l, AST_INDEX i);
extern int pe_debug;
extern FILE *pe_debugfp;
extern void pe_dump_cdg(ControlDep *cd, cdNode *cn, ASTtoPerfMapList *smap,
		   char *str);
extern int pe_get_subscript_num_dims(AST_INDEX subs);
extern void pe_get_generic_info(AST_INDEX node, int *typ, int *nargs);
extern int pe_get_type_of_binary_operator(AST_INDEX node);

#endif /* ip_perfwalk_h */
