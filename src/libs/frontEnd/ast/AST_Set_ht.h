/* $Id: AST_Set_ht.h,v 1.5 1997/03/11 14:29:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AST_Set_ht_h
#define AST_Set_ht_h

/**********************************************************************
 * 1/29/93 RvH: wait until templates are supported ...
 *
 * Hash table which maps AST_INDEX's to Sets thereof.
 *
 * Can be used, for example, for tacking a list of predecessors to an
 * AST_INDEX. 
 */

/**********************************************************************
 * Revision History:
 * $Log: AST_Set_ht.h,v $
 * Revision 1.5  1997/03/11 14:29:16  carr
 * newly checked in as revision 1.5
 *
 * Revision 1.5  94/02/27  18:31:33  reinhard
 * Added explicit constructors to make CC happy.
 * 
 * Revision 1.4  93/12/17  14:25:13  rn
 * made include paths relative to the src directory.
 * 
 * Revision 1.3  93/09/30  14:25:09  curetonk
 * changes to make AST_Set_ht.h ANSI-C compliant
 * 
 * Revision 1.2  93/06/23  10:31:21  reinhard
 * Updated comments.
 * 
 */

#ifndef AST_Set_h
#include <libs/frontEnd/ast/AST_Set.h>
#endif

#ifndef AST_ht_h
#include <libs/frontEnd/ast/AST_ht.h>
#endif

/*********************************************************************/
/*** Declaration of class AST_Set_ht *********************************/
/*********************************************************************/
class AST_Set_ht : private AST_ht {
public:
  AST_Set_ht() : AST_ht() {};

  AST_Set_ht(Data_gen_ftype my_gen_func)
    : AST_ht(my_gen_func) {};

  AST_Set_ht(Data_gen_ftype my_gen_func, Generic my_gen_arg)
    : AST_ht(my_gen_func, my_gen_arg) {};

  void add_entry(AST_INDEX key_node, AST_Set *data) {
    AST_ht::add_entry(key_node, (Generic) data);
  }
  
  Boolean query_entry(AST_INDEX query_node) {
    return (Boolean) AST_ht::query_entry(query_node);
  }
  
  AST_Set *gen_entry_by_AST(AST_INDEX query_node) {
    return (AST_Set *) AST_ht::gen_entry_by_AST(query_node);
  }

  AST_Set *gen_entries_by_ASTs(const AST_Set &nodes);

  AST_Set *recur_gen_entry_by_AST(AST_INDEX  node);

};

#endif
