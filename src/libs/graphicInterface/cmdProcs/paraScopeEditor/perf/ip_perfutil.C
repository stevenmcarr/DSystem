/* $Id: ip_perfutil.C,v 1.6 1997/03/11 14:32:10 carr Exp $*/
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * Utility routines for use in ip_perfwalk.C.
 */

/* Revision History:
 *
 * $Log: ip_perfutil.C,v $
 * Revision 1.6  1997/03/11 14:32:10  carr
 * newly checked in as revision 1.6
 *
Revision 1.6  93/12/17  14:55:45  rn
made include paths relative to the src directory. -KLC

Revision 1.5  93/08/11  16:36:37  mcintosh
Misc. cleanup; change data structure used for mapping
AST nodes to performance estimates. 

 * Revision 1.4  93/06/30  16:53:59  johnmc
 * update w.r.t. changes in sllist interface;
 * fix one bug in computing operator types
 * 
 * Revision 1.3  93/06/11  15:00:30  patton
 * made changes to allow compilation on Solaris' CC compiler
 * 
 * Revision 1.2  93/05/25  15:57:50  curetonk
 * *** empty log message ***
 * 
 * Revision 1.1  93/04/28  10:35:03  mcintosh
 * Initial revision
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/perf/ip_perfwalk.h>
#include <stdio.h>

/* ------- Methods for class PerfWalkList ---------- */

PerfWalkListEntry::PerfWalkListEntry(AST_INDEX new_ai,
				     cdNode *new_cdn,
				     PerfEstExprHandle e) 
    : CDGWalkListEntry (new_ai, new_cdn)
{
  expr = e;
}

PerfWalkListEntry::~PerfWalkListEntry()
{
  delete expr;
}

void PerfWalkList::append_entry(PerfWalkListEntry *e)
{
  CDGWalkList::append_entry((CDGWalkListEntry *) e);
}

void PerfWalkList::create_and_append_item(AST_INDEX ai,
					  cdNode *cdn,
					  PerfEstExprHandle e)
{
  PerfWalkListEntry *n = new PerfWalkListEntry(ai, cdn, e);
  append_entry(/*(CDGWalkListEntry *)*/ n);
}

/* ------- Methods for class PerfWalkListIterator ---------- */

PerfWalkListIterator::PerfWalkListIterator(PerfWalkList *l,
					   cdNode *src,
					   CDGWalkList_control cntrl)
    : CDGWalkListIterator ((CDGWalkList *) l, src, cntrl)
{
}

PerfWalkListEntry *
PerfWalkListIterator::next_entry()
{
  return (PerfWalkListEntry *) CDGWalkListIterator::next_entry();
}

PerfWalkListEntry *
PerfWalkListIterator::current()
{
  return (PerfWalkListEntry *) CDGWalkListIterator::current();
}

/* ------- Methods for class PerfLoopStackEntry ---------- */

PerfLoopStackEntry::PerfLoopStackEntry(AST_INDEX nnode,
				       char *nindvar,
				       Boolean ntriangular,
				       PerfLoopGuess nguess) :
  node(nnode), indvar(ssave(nindvar)), triangular(ntriangular),
  guess(nguess)			       
{
}

PerfLoopStackEntry::~PerfLoopStackEntry()
{
  sfree(indvar);
  if (guess.varg)
    sfree(guess.varg);
}

/* ------- Methods for class PerfLoopStack ---------- */

PerfLoopStack::PerfLoopStack()
{
}

PerfLoopStack::~PerfLoopStack()
{
}

void PerfLoopStack::push_entry(PerfLoopStackEntry *e)
{
  SinglyLinkedList::Push((SinglyLinkedListEntry *) e);
}

PerfLoopStackEntry *PerfLoopStack::pop_entry()
{
   return (PerfLoopStackEntry *) SinglyLinkedList::Pop();
}

PerfLoopStackEntry *PerfLoopStack::first_entry()
{
   return (PerfLoopStackEntry *) SinglyLinkedList::First();
}

/*---------------*/

/*
 * Control dependence graphs are sometimes hard to visualize, so I've
 * written a bunch of utility functions which print out information
 * about the graph. The function below is designed to print out the
 * control dependence graph. It gets called by "cdg_walk_nodes"; it prints
 * out each node in the control dependence graph along with a list of
 * edges, resulting in a sort of adjanceny-list display of the graph.
 * 
 * This is a fairly important debugging function.
 */

static int cd_walk_debug_post_action(cdNode *node, ASTtoPerfMapList *maplist)
{
  Generic type = ast_get_node_type(node->stmt);
  char buf[16];
  cdEdge *edge;
  double ret_et;

  ret_et = pe_debug_map_stmt_to_est(maplist, node->stmt);
  
  fprintf(pe_debugfp, "node=[%07d] est=%8.2lf %-12s",
	 node->stmt,
	 ret_et,
	 ast_get_node_type_name(type));
  if (node->succ) {
    fprintf(pe_debugfp, "\t(edge list: ");
    for (edge = node->succ; edge != NULL; edge = edge->next_succ) {
      sprintf(buf, "%d", edge->label);
      fprintf(pe_debugfp, "%s%d ",
	      edge->label == CD_FALSE ? "F" :
	      (edge->label == CD_TRUE ? "T" : 
	       buf),
	      edge->sink->stmt);
    }
    fprintf(pe_debugfp, ")\n");
  } else fprintf(pe_debugfp, "\n");
  return WALK_CONTINUE;
}

void pe_dump_cdg(ControlDep *cd, cdNode *cn,
		 ASTtoPerfMapList *smap, char *str)
{
  Generic smap_generic = (Generic) smap;
  fprintf(pe_debugfp, "%s:\n", str);
  cdg_walk_nodes(cd, cn, 0,
		 (cdg_action_callback) cd_walk_debug_post_action,
		 smap_generic);
  fflush(pe_debugfp);
}

/*
 * Find the number of arguments used for an invocation of a FORTRAN
 * generic function. This is necessary because some generics (ex: 'max')
 * take a variable number of arguments.
 */

void pe_get_generic_info(AST_INDEX node, int *typ, int *nargs)
{
  AST_INDEX alist;
  
  *typ = TYPE_REAL;
  *nargs = 0;
  alist = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  if (alist != AST_NIL)
    *typ = ast_get_converted_type(alist);
  for (; alist != AST_NIL; alist = list_next(alist))
    (*nargs)++;
}

/*
 * For a given subscripted array refernence, return the number of
 * dimensions in the reference.
 */

int pe_get_subscript_num_dims(AST_INDEX subs)
{
  AST_INDEX sublist;
  int dims;
  
  dims = 0;
  sublist = list_first(gen_SUBSCRIPT_get_rvalue_LIST(subs));
  for (dims = 0; sublist != AST_NIL; sublist = list_next(sublist))
    dims++;
  return dims;
}

/*
 * The type checker figures out what type operators are, isn't that nice?
 * Yes sir, it is. Can you say, "that's nice"? Good, I thought you could.
 * 
 * Sometimes there are typechecker bugs, though, so we still need to cover
 * our bets.
 */

int pe_get_type_of_binary_operator(AST_INDEX node)
{
  AST_INDEX op1, op2;
  int nsons;
  
  if (gen_get_converted_type(node) == TYPE_UNKNOWN) {
    /*
     * Try to get the type from the two sons.
     */
    nsons = gen_how_many_sons(gen_get_node_type(node));
    if (nsons == 2) {
      op1 = ast_get_son_n(node, 1);
      if (gen_get_converted_type(op1) != TYPE_UNKNOWN)
	return gen_get_converted_type(op1);
      op2 = ast_get_son_n(node, 2);
      if (gen_get_converted_type(op2) != TYPE_UNKNOWN)
	return gen_get_converted_type(op2);
    }
    return TYPE_REAL;	/* give up */
  }
  return gen_get_converted_type(node);
}
