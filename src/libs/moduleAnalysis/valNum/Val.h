/* $Id: Val.h,v 1.2 1997/03/11 14:36:17 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*****************************************************************************
 *
 *
 *  -- Val.h
 *
 *  Public C++ header file for the symbolic analysis and value
 *  numbering routines.
 *  If you're using C, include val.h instead.
 *
 *****************************************************************************/
 
#ifndef Val_h
#define Val_h
 
#ifndef val_h
#include <libs/moduleAnalysis/valNum/val.h>
#endif

/*********************************************************************
 *
 * Value Number Iterator
 *
 * Usage:
 *
 * // Eg., "vns = val_get_subs(val, valNum);" for an array <val>
 *
 * // Loop over value numbers of <vns>:
 * for (ValueIterator iter(val, vns); (cur = iter()) != VAL_NIL;)
 * { ... }
 */
class ValueIterator {
  Generic hidden;    // Cached hidden internal representation of values
  int     arity;     // # of values
  int     current;   // Iterator counter
 
public:
  ValueIterator(Values val, ValNumber vns);  // Constructor
  ValNumber operator() ();                   // Step
  void      reset();                         // Reset to beginning
};

#endif
