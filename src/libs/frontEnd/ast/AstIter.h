/* $Id: AstIter.h,v 1.5 1997/03/27 20:34:58 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*********************************************************************
 *
 * class AstIter: An iterator walking through an AST.
 *
 * Usage:
 *
 * AST_INDEX node;
 * for (AstIter iter(start_node); (node = iter()) != AST_NIL;)
 * {
 *   // do something w/ node
 * }
 *
 * After finishing the walk, the iterator resets itself to the start,
 * so the same iterator can be used to walk the same tree multiple
 * times.
 * Alternativly, for example after prematurely finishing a previous
 * walk, the iterator can be reset() and then be used for a new walk.
 *
 * By default, the iterator considers only stmt levels at the same or
 * deeper level as <start_node>; however, if <only_inner> is false, then
 * the iterator walks also outwards on the AST; ie, until the end of
 * the program/procedure/etc.  In this case, <level> might become < 0.
 *
 * Also by default, the interator walks at the statement level only;
 * hewever, if <only_stmts> is false, then we explore the AST down to
 * the leaf (expression) level.
 *
 * NOTE: When the current node is modified, put_cur_node() has to be
 *       called w/ the new node.
 */
#ifndef AstIter_h
#define AstIter_h

#ifndef astlist_h
#include <libs/frontEnd/ast/astlist.h>
#endif

#ifndef treeutil_h
#include <libs/frontEnd/ast/treeutil.h>
#endif

class AstIter {
public:
  // Constructor
  AstIter(AST_INDEX my_start_node,        // Where to start in AST
	  Boolean my_only_stmts = true,   // Statement level by default
	  Boolean my_only_inner = true);  // No outer nodes by default
  
  AST_INDEX operator() ();                // Advance operator
  void      putCur_node(AST_INDEX my_cur_node); // Update cur_node
  void      reset();                      // Reset iterator to start

private:
  AST_INDEX lookahead_node(AST_INDEX node);
  AST_INDEX next_stmt_or_expr(AST_INDEX node);
  AST_INDEX next_son(AST_INDEX father_node, int prev_son = 0);

  // Private fields
  AST_INDEX start_node;  // Node where we started
  AST_INDEX cur_node;    // Current node
  AST_INDEX next_node;   // Next node
  int       level;       // Current nesting level
  Boolean   only_stmts;  // Not below statement level ?
  Boolean   only_inner;  // Do we also move out on the tree ?
};

#endif
