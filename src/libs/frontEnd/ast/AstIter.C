/* $Id: AstIter.C,v 1.4 2001/10/12 19:37:04 carr Exp $ */
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
 */

#include <iostream.h>
#include <libs/frontEnd/ast/AstIter.h>

#ifndef asttree_h
#include <libs/frontEnd/ast/asttree.h>
#endif

#ifndef astutil_h
#include <libs/frontEnd/ast/astutil.h>
#endif

/**********************************************************************
 * Constructor
 */
AstIter::AstIter(AST_INDEX my_start_node,
		 Boolean   my_only_stmts,
		 Boolean   my_only_inner) 
: start_node (my_start_node),
  cur_node   (AST_NIL),
  next_node  (lookahead_node(AST_NIL)),
  only_stmts (my_only_stmts),
  only_inner (my_only_inner)
{
}


/**********************************************************************
 * Advance operator
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
 * If we are handed a list of stmts, we walk all elements of that list.
 */
AST_INDEX
AstIter::operator() ()
{      
  cur_node = lookahead_node(cur_node);

  return cur_node;
}


/**********************************************************************
 * putCur_node()
 */
void
AstIter::putCur_node(AST_INDEX my_cur_node)
{
  cur_node = my_cur_node;
}


/**********************************************************************
 * reset()
 */
void
AstIter::reset()
{
  cur_node = AST_NIL;
}


/**********************************************************************
 * lookahead_node()
 */
AST_INDEX
AstIter::lookahead_node(AST_INDEX node)
{
  AST_INDEX inner_node;
  
  if (node == AST_NIL)                // At start of walk ?
  {
    if (is_list(start_node))          // Are we handed a list ?
    {
      node = list_first(start_node);  // Start at beginning of list
      level= 1;                       // at level 1
    }
    else
    {
      node  = start_node;             // Else, start at node
      level = 0;                      // at level 0
    }
  }
  else
  {
    inner_node = only_stmts ? gen_get_stmt_list(node)
      : next_son(node);
    if (inner_node != AST_NIL)                  // Inner nodes present ?
    {
      // 9/22/93 RvH: Note that some nodes (eg, WRITE) have several
      //              lists as sons
     if (is_list(inner_node))                  // Walk statement list
      {
	level = level + 2;                
	node  = list_first(inner_node);
      }
      else
      {
	level++;                
	node = inner_node;
      }
    }
    else
    {
      while ((node != AST_NIL)                  // Walk up if necessary
	     && (next_stmt_or_expr(node) == AST_NIL)
	     && ((level-- > 0) || (!only_inner)))
      {
	//node = tree_out(node);
	node = ast_get_father(node);
      }
      if (node != AST_NIL)                     // Is there a next node ?
      {
	node = ((level > 0) || (!only_inner))
	  ? next_stmt_or_expr(node) : AST_NIL;
      }
    }
  }

  return node;
}


/**********************************************************************
 * next_stmt_or_expr()
 */
AST_INDEX
AstIter::next_stmt_or_expr(AST_INDEX node)
{
  int       which_son;
  AST_INDEX father_node;
  AST_INDEX next_node = AST_NIL;

  if (in_list(node))
  {
    next_node = list_next(node);
  }
  else
  {
    if (!only_stmts)                       // Not only statement level ?
    {
      father_node = ast_get_father(node);

      if (father_node != AST_NIL)
      {
	which_son   = ast_which_son(father_node, node);
	next_node   = next_son(father_node, which_son);

	if (is_list(next_node))
	{
	  level++;
	  next_node = list_first(next_node);
	}
      }
    }
  }

  return next_node;
}


/**********************************************************************
 * next_son()
 */
AST_INDEX
AstIter::next_son(AST_INDEX father_node, int prev_son)
{
  AST_INDEX son_node = AST_NIL;
  int       num_of_sons, which_son;

  num_of_sons = ast_get_son_count(father_node);
  for (which_son = prev_son + 1;
       (which_son <= num_of_sons) && (son_node == AST_NIL);
       which_son++)
  {
    son_node = ast_get_son_n(father_node, which_son);
  }

  return son_node;
}
