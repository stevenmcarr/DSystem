/* $Id: AST_Set.h,v 1.4 1997/03/11 14:29:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef AST_Set_h
#define AST_Set_h

/**********************************************************************
 * Set of AST_INDEX's.
 */
/**********************************************************************
 * Revision history:
 * $Log: AST_Set.h,v $
 * Revision 1.4  1997/03/11 14:29:16  carr
 * newly checked in as revision 1.4
 *
 * Revision 1.4  93/12/17  14:25:12  rn
 * made include paths relative to the src directory.
 * 
 * Revision 1.3  93/09/30  14:25:03  curetonk
 * changes to make AST_Set.h ANSI-C compliant
 * 
 * Revision 1.2  93/06/23  10:31:13  reinhard
 * Updated comments.
 * 
 */

#ifndef Sparse_Set_h
#include <libs/support/sets/Sparse_Set.h>
#endif

#ifndef ast_h
#include <libs/frontEnd/ast/ast.h>
#endif

/*********************************************************************/
/*** Declaration of class AST_Set ************************************/
/*********************************************************************/
class AST_Set : public Sparse_Set {
public:
  AST_Set() : Sparse_Set() {};
  ~AST_Set() {};

  Boolean query_entry(AST_INDEX node) const {
    return Sparse_Set::query_entry((Element) node);
  }
  
  int get_entry_index(AST_INDEX node) const {
    return Sparse_Set::get_entry_index((Element) node);
  }
  
  AST_INDEX get_entry_by_index(int index) const {
    return (AST_INDEX) Sparse_Set::get_entry_by_index(index);
  }
  
  AST_Set& operator+= (const AST_INDEX& node) {
    return (AST_Set&) Sparse_Set::operator+=((const Element&) node);
  }  

  AST_Set& operator-= (const AST_INDEX& node) {
    return (AST_Set&) Sparse_Set::operator-=((const Element&) node);
  }  

  AST_Set& operator+= (const AST_Set& other_set) {
    return (AST_Set&) Sparse_Set::operator+=((const Sparse_Set&)
					     other_set);
  }
};

/**********************************************************************
 * An Iterator.
 * Usage:
 *    AST_Set      set;
 *    AST_INDEX    node;
 *    AST_Set_Iter iter(set);
 *    while (node = iter()) { ... }
 */
class AST_Set_Iter : private Sparse_Set_Iter {
public:
  AST_Set_Iter(const AST_Set& my_set)
    : Sparse_Set_Iter((const Sparse_Set&) my_set) {}

  AST_INDEX operator() () {
    return (AST_INDEX) Sparse_Set_Iter::operator() ();
  }
};

#endif
