/* $Id: AST_Graph.C,v 1.3 1997/03/11 14:28:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/**********************************************************************
 * Class for AST_INDEX based accesses to AST related graphs, such as
 * the CFG/SSA graph.
 *
 * This is meant to be a base class from which classes specific to
 * a particular kind of graph can be derived.
 *
 * For now, just access to immediate predecessors/successors and the
 * transitive closure thereof.
 * These accesses return Sets of AST_INDEX's.
 * The results of each access are cached in hashtables mapping
 * AST_INDEX's to sets thereof.
 */
/**********************************************************************
 * Revision history:
 * $Log: AST_Graph.C,v $
 * Revision 1.3  1997/03/11 14:28:23  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  94/03/21  12:39:03  patton
fixed comment problem

Revision 1.2  94/02/27  20:13:40  reinhard
Added value-based distributions.
Make CC happy.
See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.

Revision 1.1  1994/02/27  19:36:22  reinhard
Initial revision

 */

#include <libs/fortD/irregAnalysis/AST_Graph.h>

/**********************************************************************
 * Constructor
 */
AST_Graph::AST_Graph(Data_gen_ftype my_walk_func,
		     Generic        my_graph) 
: walk_func(my_walk_func),
  graph (my_graph),
  ipreds_ht(new AST_Set_ht(walk_func, graph))
{
}


/**********************************************************************
 * Constructor
 */
AST_Graph::AST_Graph(Data_gen_ftype my_walk_func)
: walk_func(my_walk_func)
{
}


/**********************************************************************
 * ipreds()   Map <node> to its immediate predecessors
 */
AST_Set *
AST_Graph::ipreds(AST_INDEX node)
{
  return ipreds_ht->gen_entry_by_AST(node);
}


/**********************************************************************
 * ipreds()   Map <nodes> to their immediate predecessors
 */
AST_Set *
AST_Graph::ipreds(const AST_Set &nodes)
{
  return ipreds_ht->gen_entries_by_ASTs(nodes);
}


/**********************************************************************
 * preds()  Map <node> to its predecessors
 */
AST_Set *
AST_Graph::preds(AST_INDEX node)
{
  return ipreds_ht->recur_gen_entry_by_AST(node);
}
