/* $Id: cdgwalklist.C,v 1.9 1997/03/11 14:32:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * $Id
 * 
 * Utility routines for maintaining a list of recently visited nodes during a
 * bottom-up CDG walk. See the corresponding include file for more information.
 */

/* Revision History:
 *
 * $Log: cdgwalklist.C,v $
 * Revision 1.9  1997/03/11 14:32:07  carr
 * newly checked in as revision 1.9
 *
 * Revision 1.9  93/12/17  14:55:37  rn
 * made include paths relative to the src directory. -KLC
 * 
 * Revision 1.8  93/08/04  21:23:22  mcintosh
 * Fix some bugs (apparently caused by the C++ compiler). 
 * 
 * Revision 1.7  1993/06/30  16:53:21  johnmc
 * update w.r.t. changes in sllist interface
 *
 * Revision 1.6  93/06/11  14:58:45  patton
 * made changes to allow compilation on Solaris' CC complier
 * 
 * Revision 1.5  92/12/14  21:55:33  mcintosh
 * Improve comments (no changes to functionality). 
 * 
 *
 */

#include <assert.h>
#include <stdio.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/cdgwalklist.h>

/* Hack used for building temporary lists of pointers. 
*/

class PointerListEntry : private SinglyLinkedListEntry {
 public:
  void *data;
  PointerListEntry(void *d) { data = d; };
};

/* -------- Methods for class CDGWalkListEntry -------- */

CDGWalkListEntry::CDGWalkListEntry(AST_INDEX new_ai, cdNode *new_cdn)
{
  cdEdge *edge;

  ai = new_ai;
  cdn = new_cdn;
  refcount = 0;

  /* Count the incoming edges for this node
  */
  if (cdn) {
    refcount = 0;
    for (edge = cdn->pred; edge; edge = edge->next_pred) 
      refcount++;
  } else refcount = 1;
}

CDGWalkListEntry::~CDGWalkListEntry()
{
}


/* -------- Methods for class CDGWalkList -------- */

void CDGWalkList::create_and_append_item(AST_INDEX ai, cdNode *cdn)
{
  CDGWalkListEntry *n = new CDGWalkListEntry(ai, cdn);
  append_entry(n);
}

void CDGWalkList::append_entry(CDGWalkListEntry *e)
{
  SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
}

void CDGWalkList::delete_entry(CDGWalkListEntry *e)
{
  SinglyLinkedList::Delete((SinglyLinkedListEntry *) e);
}

CDGWalkList::CDGWalkList()
{
}

CDGWalkList::~CDGWalkList()
{
}

/*
 * We want to avoid iterating through a list we are in the process of
 * destroying, so do this in two passes: build a list of the elements we
 * want to delete, then iterator through the built list, getting rid of
 * the selected elements.
 * 
 * The techniques in this method result in O(N^2) complexity. If speed is an
 * issue, someone should recode this in a more efficent manner.
 */

void CDGWalkList::prune_list(cdNode *source_node,
			       CDGWalkList_control prune_control,
			       Boolean use_refcounting)
{
  PointerListEntry *n;
  SinglyLinkedList l;
  CDGWalkListEntry *cur;
  CDGWalkListIterator i(this, source_node, prune_control);
  
  for (; cur = i.current(); cur = i.next_entry()) {
    cur->refcount--;
    if (prune_control == ALL_LIST_ELEMENTS || !use_refcounting ||
	cur->refcount == 0) {
      n = new PointerListEntry((void *) cur);
      l.Append((SinglyLinkedListEntry *) n);
    }
  }

  {
    SinglyLinkedListIterator li(&l);
    for (; n = ((PointerListEntry *) (li.Current())); li++) {
      delete_entry((CDGWalkListEntry *) n->data);
    }
  }
}
       
/* -------- Methods for class CDGWalkListIterator -------- */

CDGWalkListIterator::CDGWalkListIterator(CDGWalkList *l, cdNode *src,
					 CDGWalkList_control cntrl)
{
  i = new SinglyLinkedListIterator((SinglyLinkedList *) l);
  control = cntrl;
  source = src;
}

CDGWalkListIterator::~CDGWalkListIterator()
{
  delete i;
}

CDGWalkListEntry *CDGWalkListIterator::current()
{
  bypass_inappropriate_entries();

  return (CDGWalkListEntry *) i->Current();
}

CDGWalkListEntry *CDGWalkListIterator::next_entry()
{
  (void) (*i)++;
  bypass_inappropriate_entries();

  return (CDGWalkListEntry *) i->Current();
}

void CDGWalkListIterator::bypass_inappropriate_entries()
{
  CDGWalkListEntry *cur = (CDGWalkListEntry *) (i->Current());

  if (!cur)
    return;
  
  /* Get the next element which meets the criteria.
  */
  switch(control) {
    case ALL_LIST_ELEMENTS:
    {
      break;
    }
    case ALL_CONTROLDEP_LIST_ELEMENTS:
    {
      cdEdge *edge;
      int found = 0;

      while (cur && !found) {
	for (edge = cur->cdn->pred; !found && edge != NULL;
	     edge = edge->next_pred) {
	  if (edge->src == source)
	    found = 1;
	}
	if (!found) {
	  (*i)++; // advance iterator to next element
	  cur = (CDGWalkListEntry *) i->Current();
	}
      }
      break;
    }
    case ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS:
    case ONLY_FALSE_CONTROLDEP_LIST_ELEMENTS:
    {
      cdEdge *edge;
      int found = 0;
      int edgelabel;

      if (control == ONLY_TRUE_CONTROLDEP_LIST_ELEMENTS)
	edgelabel = CD_TRUE;
      else 
	edgelabel = CD_FALSE;

      while (cur && !found) {
	for (edge = cur->cdn->pred; !found && edge != NULL;
	     edge = edge->next_pred) {
	  if (edge->src == source && edge->label == edgelabel)
	    found = 1;
	}
	if (!found) {
	  (*i)++; // advance iterator to next element
	  cur = (CDGWalkListEntry *) i->Current();
	}
      }
      break;
    }
  }
}
