/* $Id: cdgwalklist.h,v 1.7 1997/03/11 14:32:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * This file is the public interface to the CDGwalkList class. This class is
 * designed to provide help for problems which involve bottom-up walks
 * (i.e. using cdg_walk_nodes()) of the control dependence graph for a
 * procedure.
 * 
 * In a bottom-up walk of the control dependence graph, for a given node X,
 * you first visit all of the nodes { Y1, Y2, ... Yn } which are
 * control-dependent on X, and then you visit X (ignoring back edges). It
 * is very often the case that when you visit the Yn nodes, you want to
 * perform some computation on each of the Y's and store the results in a
 * data structure, so that when you reach X, you can get the results of
 * the computation again for each of the Y's.
 * 
 * This data structure is an attempt to provide such a facility. The main
 * abstraction is a list. When an item Y is visited, a node for it is
 * added to the list. When Y's parent in the control-dependence graph X is
 * visited, X can search the list for the node corresponding to Y.
 *
 * For example, consider the following ridiculously contrived application.
 * Suppose for each node in the control dependence graph, I want to print
 * out the line number for that node's statement, the line numbers of all
 * the "DO" loops which are immediately control-dependent on that node,
 * and the total number of "DO" loops which are control-dependent in any
 * way (i.e.  not just immediately control-dependent) on that node. Here
 * is a simple program along with the output that I need:
 *
 * 1     program mumble
 * 2     do i=1,10
 * 3       if (x) then
 * 4         do j=1,100
 * 5           proc1(x)
 * 6         enddo
 * 7       endif
 * 8     enddo
 * 
 * Output:
 * 
 * 1: immediate=(2) total=2
 * 2: immediate=(0) total=1
 * 3: immediate=(1) total=1
 * 4: immediate=(0) total=0
 * ...
 * 8: immediate=(0) total=0
 * 
 * To accomplish this task using CDGwalkList, I would do the following.
 * First, create a subclass of CDGwalkListEntry (call it
 * LoopCDwalkListEntry) with an additional data element, an integer called
 * "childLoops". Then I would call cdg_walk_nodes on the control
 * dependence graph for the procedure, passing it a pointer to a newly
 * created LoopCDwalkList. The POST_ACTION callback for the walk would do
 * the following:
 * 
 * 1) iterate through the elements on the LoopCDwalkList, summing the
 * "childLoops" fields for each node, and printing out any immediately
 * control-dependent loops,
 * 
 * 2) print the sum calculated in the previous step
 * 
 * 3) prune the list of control-dependent items using prune_list of the list
 * which correspond to are immediately control-dependent items,
 * 
 * 4) add an item corresponding to the current node to the list, setting the
 * "childLoops" field to the previously calculated sum
 * 
 * This would accomplish the desired effect.
 * 
 * Author: Nat McIntosh
 * 
 */

/* Revision History:
 * $Log: cdgwalklist.h,v $
 * Revision 1.7  1997/03/11 14:32:07  carr
 * newly checked in as revision 1.7
 *
 * Revision 1.7  93/12/17  14:55:39  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.6  93/06/11  13:40:35  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.5  93/04/28  10:44:58  mcintosh
 * Give CDGWalkList a virtual destructor, just to be sure. 
 * 
 * Revision 1.4  92/12/14  21:54:55  mcintosh
 * Improve comments (no changes to functionality). 
 * 
 * 
 */

#ifndef cdgwalklist_h
#define cdgwalklist_h

/* ----- Include files ----- */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef fort_h
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/fort.h>
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

#include <libs/support/lists/SinglyLinkedList.h>

/* ----- Misc. enumerations, etc  ----- */

/*
 * The following enumeration is used to control the routines for visiting
 * particular elements of a list, and for pruning the list.
 */

typedef enum _CDGWalkList_control {
  /*
   * Remove all items from the list
   */
  ALL_LIST_ELEMENTS=0,

  /*
   * Remove items corresponding to nodes which are control dependent on a
   * specified node.
   */
  ALL_CONTROLDEP_LIST_ELEMENTS,

  /*
   * Remove items corresponding to nodes which are TRUE control dependent
   * on a specified node.
   */
  ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS,

  /*
   * Remove items corresponding to nodes which are FALSE control dependent
   * on a specified node.
   */
  ONLY_FALSE_CONTROLDEP_LIST_ELEMENTS

} CDGWalkList_control;
  
/* ----- C++ class definitions  ----- */

/*
 * Class for items on a list. This class is designed not to be used as is,
 * but to be inherited from (i.e. you create a subclass which has more
 * data).
 */

class CDGWalkListEntry : private SinglyLinkedListEntry {
 public:
  AST_INDEX ai;		/* AST index for this item */
  cdNode *cdn;		/* CDG node for this item */
  int refcount;		/* in-edges for this node */

  CDGWalkListEntry(AST_INDEX new_ai, cdNode *new_cdn);
  ~CDGWalkListEntry();
};

/*
 * Class for the list itself. You can create a subclass of this class just
 * to avoid warnings about using the methods with incorrect argument
 * types.
 */

class CDGWalkList : public SinglyLinkedList {
  
 public:

  CDGWalkList();
  virtual ~CDGWalkList();

  /*
   * Override various existing methods from the parent class so we can use
   * them it without getting argument-type warnings.
   */
   void append_entry(CDGWalkListEntry *e);
   void delete_entry(CDGWalkListEntry *e);

   /* Shorthand for the routine above (for convenience only). 
   */
   void create_and_append_item(AST_INDEX ai, cdNode *cdn);

  /*
   * Purge the list of certain items.
   * 
   * Note that a node X may be control-dependent on more than 1 other node
   * (i.e. have more than 1 in-edge). In this case, the
   * "use_refcounting" option can be used to only remove nodes from the
   * list when their reference count reaches 0. For example, consider
   * the following application. Suppose we want to write a program which
   * visits each statement X in the program, and at each statement
   * prints out the message "Y, Z, W, ... are control dependent on X",
   * where Y, Z, W, etc. are statements which are control dependent on
   * X. Consider the following piece of code.
   * 
   *
   *		subroutine mumble(x,y)
   *	10	  if (x .lt y) goto 30
   *	20	  if (x .eq. y) return
   *	30	x = 1
   *		return
   *
   *
   * The assignment statement will be control-dependent on *both* if
   * statements. Suppose that we do a bottom-up walk of the CDG, where each
   * time we visit a node X, we add X to a CDGWalkList. Then we visit each
   * node N in the list and print "N is control dependent on X". Finally, we
   * remove all the nodes we just visited (i.e. the nodes we printed
   * messages about). Well, when we visit statement 20, the node
   * corresponding to statement 30 would be removed from the list, since
   * it's control-dependent on 20. But 30 is also control-dependent on 10!
   * When we visit node 20, we can't remove node 30 from the list, since we
   * want to be able to print that it's also control-dependent on 10.
   * 
   * To deal with this problem, set the parameter "use_refcounting" to 'true'.
   * In this case (unless control == ALL_LIST_ELEMENTS), before deleting an
   * element from the list, we decrement it's refcount, and then we only
   * delete it if its refcount is 0.
   * 
   */
   void prune_list(cdNode *source_node, CDGWalkList_control purge_control,
		   Boolean use_refcounting);

   /* use CDGWalkListIterator for traversing the list */
};

/* Class for iterating through a CDGwalkList. 
   Usage:

   CDGWalkListIterator i(..params..);
   CDGWalkListEntry *cur;

   for (; cur = i.current(); cur = i.next_entry()) {
     ... code to loop at "cur" ...
     }
*/

class CDGWalkListIterator {

  /* Determines which items get visited 
  */
  CDGWalkList_control control;
  SinglyLinkedListIterator *i;
  cdNode *source;
  void bypass_inappropriate_entries();

 public:
  CDGWalkListIterator(CDGWalkList *l, cdNode *src,
		      CDGWalkList_control cntrl);
  ~CDGWalkListIterator();
  CDGWalkListEntry *next_entry();
  CDGWalkListEntry *current();
};

#endif /* cdgwalklist_h */
