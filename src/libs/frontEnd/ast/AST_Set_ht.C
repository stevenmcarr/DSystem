/* $Id: AST_Set_ht.C,v 1.3 1997/03/11 14:29:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/**********************************************************************
 * Hash table which maps AST_INDEX's to Sets thereof.
 *
 * Can be used, for example, for tacking a list of predecessors to an
 * AST_INDEX. 
 */

/**********************************************************************
 * Revision History:
 * $Log: AST_Set_ht.C,v $
 * Revision 1.3  1997/03/11 14:29:16  carr
 * newly checked in as revision 1.3
 *
Revision 1.3  93/12/17  14:46:30  rn
made include paths relative to the src directory. -KLC

Revision 1.2  93/10/19  14:24:58  curetonk
minor changes

 * Revision 1.1  93/06/22  18:00:54  reinhard
 * Initial revision
 * 
 * Revision 1.2  1993/06/09  23:42:46  reinhard
 * Cleaned up include hierarchy.
 *
 * Revision 1.1  1993/02/19  17:58:36  reinhard
 * Initial revision
 *
 */

#include <assert.h>

#ifndef AST_Set_ht_h
#include <libs/frontEnd/ast/AST_Set_ht.h>
#endif

/**********************************************************************
 * gen_entries_by_ASTs()  Generate sets for the given nodes and merge
 *                        them.
 */
AST_Set *
AST_Set_ht::gen_entries_by_ASTs(const AST_Set  &nodes)
{
  AST_Set      *result, *merged_results;
  AST_Set_Iter nodes_iter(nodes);
  AST_INDEX    node;

  merged_results = new AST_Set;
  while (node = nodes_iter())
  {
    result          = gen_entry_by_AST(node);
    *merged_results += *result;
    delete result;
  }
  
  return merged_results;
}


/**********************************************************************
 * recur_gen_entry_by_AST()  Generate AST_Set's recursively
 *                           (acts like a transitive closure).
 */
AST_Set *
AST_Set_ht::recur_gen_entry_by_AST(AST_INDEX node) 
{
  AST_Set *prev_results, *results, *next_results;
  int      cnt;
  
  prev_results = gen_entry_by_AST(node);
  
  do {
    results      = prev_results;
    cnt          = results->count();
    next_results = gen_entries_by_ASTs(*results);
    *results     += *next_results;
  } while (cnt != results->count());
  
  return results;
}
