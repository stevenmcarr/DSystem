/* $Id: AST_Graph.h,v 1.9 1997/03/11 14:28:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AST_Graph_h
#define AST_Graph_h

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
 * $Log: AST_Graph.h,v $
 * Revision 1.9  1997/03/11 14:28:24  carr
 * newly checked in as revision 1.9
 *
 * Revision 1.9  94/03/21  12:53:07  patton
 * fixed comment problem
 * 
 * Revision 1.8  94/02/27  20:14:09  reinhard
 * Added value-based distributions.
 * Make CC happy.
 * See /home/reinhard/rn/zzzgroup_src/libs/fort_d/irreg files for details.
 * 
 * Revision 1.6  1994/02/27  19:37:17  reinhard
 * Class for AST_INDEX based accesses to AST related graphs, such as
 * the CFG/SSA graph.
 *
 * Revision 1.5  1994/01/18  19:42:24  reinhard
 * Updated include paths according to Makefile change.
 *
 * Revision 1.4  1993/06/22  23:06:53  reinhard
 * Moved include file to fort.
 *
 * Revision 1.3  1993/06/09  23:41:36  reinhard
 * Cleaned up include hierarchy.
 *
 */

#ifndef _AST_SET_HT
#include <libs/frontEnd/ast/AST_Set_ht.h>
#endif

/*------------------- FORWARD DECLARATIONS ------------------*/

class AST_Graph;


/*********************************************************************/
/*** Declaration of class AST_Graph **********************************/
/*********************************************************************/
class AST_Graph
{
protected:
  Data_gen_ftype walk_func;  // Function to walk graph in one direction
  Generic        graph;      // Graph data structure (eg., CfgInstance)
  AST_Set_ht     *ipreds_ht; // Cache for immediate predecessors

public:
  // Constructors
  AST_Graph(Data_gen_ftype my_walk_func, Generic my_graph);
  AST_Graph(Data_gen_ftype my_walk_func);
  
  // Access Functions
  Generic getGraph() { return graph; }

  // Map <node> to its immediate predecessors
  AST_Set *ipreds(AST_INDEX node);

  // Map <nodes> to their immediate predecessors
  AST_Set *ipreds(const AST_Set &nodes);

  // Map <node> to its predecessors
  AST_Set *preds(AST_INDEX node);

/*
  // Map <nodes> to their predecessors
  AST_Set *preds(const AST_Set &nodes) {
    return ipreds_ht->recur_gen_entries_by_ASTs(nodes);
  }
*/
};

#endif
