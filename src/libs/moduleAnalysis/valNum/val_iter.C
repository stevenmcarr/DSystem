/* $Id: val_iter.C,v 1.2 1997/03/11 14:36:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <assert.h>
#include <libs/moduleAnalysis/valNum/val_ht.h>
#include <libs/moduleAnalysis/valNum/Val.h>
#include <libs/moduleAnalysis/valNum/val.i>

/*********************************************************************
 *
 * ValueIterator methods
 *
 *********************************************************************/

/*********************************************************************
 *  Constructor
 */
ValueIterator::ValueIterator(Values val, ValNumber vns)
{
  ValType vn_type = val_get_val_type(val, vns);
  ValEntry &ve = (*val)[vns];   // Get the entry for this value number

  assert(vn_type == VAL_LIST);  // Sets of values are of type VAL_LIST
  hidden  = (Generic) &ve;      // Store the entry for later use
  arity   = ve_arity(ve);       // Determine the # of values
  current = 0;                  // Initialize counter
}


/*********************************************************************
 *  Step
 */
ValNumber
ValueIterator::operator() ()
{
  ValEntry *ve = (ValEntry *) hidden;
  ValNumber vn;

  if (current < arity)
  {
    vn = ve_kid(*ve, current++);
  }
  else
  {
    vn = VAL_NIL;
    reset();     // Wrap around iterator starts at first vn again
  }

  return vn;
}


/*********************************************************************
 *  Reset state to beginning.
 */
void
ValueIterator::reset()
{
  current = 0; 
}

