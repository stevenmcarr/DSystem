/* $Id: val_arith.C,v 2.8 1997/03/11 14:36:18 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 * -- val_arith.c
 * 
 *          This file contains arithmatci helper functions for value nodes
 * 
 *        Precondition:  For constants, all the following function only handle
 *                       INTEGER type.
 *        Warning:  All the functions in this file have the potential
 *                  of causing reallocation of value table.
 *                  Beware of dangling pointers!!!
 *
 *        Note:  All the following functions will take arguments of any
 *               value node type.   Function names with prefix
 *               "val_*()" will return exact result whenever possible
 *               while functions withe prefix "range_*()" will only
 *               return constant range of the result.
 *
 *   By: Po-Jen Yang                Last Modified: June 1991
 *
 *	  Now most range_* functions are eliminated; bare ranges (not
 *	  wrapped in a VAL_IVAR) are treated as "constant ranges" -- 
 *	  must have VAL_CONSTANT or VAL_BOTTOM for each of lo, hi, step,
 *	  align.				Paul Havlak, 6 April 1993
 ****************************************************************************/

#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/support/numerical/ExtendedGCD.h>

/****************************************************************************
 * -- val_sign
 *
 *    This function computes the sign of the content of a value number.
 *    The possible return values are 
 *          VAL_SIGN_ZERO: val == 0
 *          VAL_SIGN_POS:  val > 0
 *          VAL_SIGN_NEG:  val < 0
 *    If none of above is true,
 *          VAL_SIGN_NONNEG:  val >= 0
 *          VAL_SIGN_NONPOS:  val <= 0
 *          VAL_SIGN_UNKNOWN: unable to make the comparison
 *
 *    Warning: This function might cause the value table to be reallocated
 *             if valNum's constant range has not yet been built.
 ****************************************************************************/
ValSign val_sign(Values Vp,
                 ValNumber v)
{
    if ((v == VAL_BOTTOM) || (v == VAL_NIL)) return VAL_SIGN_UNKNOWN;
    if (v == VAL_TOP)			     return VAL_SIGN_ANY;

    ValNumber cRange = ve_cRange((*Vp)[v]);

    if (cRange == VAL_BOTTOM) return VAL_SIGN_UNKNOWN;

    if (ve_type((*Vp)[cRange]) == VAL_CONST)
    {
	int constNum = ve_const((*Vp)[v]);

	if (constNum < 0) 	return VAL_SIGN_NEG;
	else if (constNum == 0) return VAL_SIGN_ZERO;
	else			return VAL_SIGN_POS;
    }
    else if (ve_type((*Vp)[cRange]) == VAL_RANGE)
    {
	int hi = ve_const((*Vp)[ve_hi((*Vp)[cRange])]);
	int lo = ve_const((*Vp)[ve_hi((*Vp)[cRange])]);

	if (hi < 0)	  return VAL_SIGN_NEG;
	else if (lo > 0)  return VAL_SIGN_POS;
	else if (hi == 0) return VAL_SIGN_NONPOS;
	else if (lo == 0) return VAL_SIGN_NONNEG;
	else		  return VAL_SIGN_UNKNOWN;
    }
    //  something is wrong with constant ranges
    else die_with_message("Error in val_sign (handling of ranges)\n");

    return VAL_SIGN_UNKNOWN;
}





/****************************************************************************
 * -- val_compare
 *
 *    This function compares two constant value number node.
 *    It uses range_diff and val_sign to compute the result
 *
 *    The possible return values are 
 *          VAL_CMP_EQ:      val1 == val2
 *          VAL_CMP_LT:      val1 <  val2
 *          VAL_CMP_GT:      val1 >  val2
 *    If none of above is true,
 *          VAL_CMP_GE:      val1 >= val2
 *          VAL_CMP_LE:      val1 <= val2
 *          VAL_CMP_UNKNOWN: unable to make the comparison
 ****************************************************************************/
ValSign val_compare(Values Vp,
		    ValNumber v1,
		    ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP)) return VAL_CMP_ANY;

    if ((!val_is_value(v1)) || (!val_is_value(v2)))
    {
	return VAL_CMP_UNKNOWN;
    }

    if (v1 == v2) return VAL_CMP_EQ;

    /*
     *  Now that things are being simplified, constructing the actual
     *  difference is wasteful (results in extra dead values).
     *
     *  if both constant compare
     *  extract added constants and see if equal
     *  if one or both are ranges compare symbolic bounds
     *  compare conservatively-built constant ranges
     */
    ValNumber diff = val_binary(Vp, VAL_OP_MINUS,
				v1, v2);
    return val_sign(Vp,diff);
}



/**************************************************************************** 
 *  -- val_min
 *
 *     Return the minimum of two values, if possible.
 ****************************************************************************/

ValNumber val_min(Values Vp,
		  ValNumber v1,
		  ValNumber v2)
{
    ValSign diffSign;

    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    diffSign = val_compare(Vp,v1, v2);

    if ( VAL_is_ge(diffSign) )		return v2;
    else if ( VAL_is_le(diffSign) )	return v1;
    else				return VAL_BOTTOM;

} /* end of val_min() */





/**************************************************************************** 
 *  -- val_max
 *
 *     Return the maxmum of two value number entry.
 *     VAL_TOP is returned when either v1/v2 is TOP
 *     else VAL_BOTTOM is returned when order cannot be determined
 *
 ****************************************************************************/

ValNumber val_max(Values Vp,
		  ValNumber v1,
		  ValNumber v2)
{
    ValSign diffSign;

    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))	    return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    diffSign = val_compare(Vp,v2, v1);

    if ( VAL_is_ge(diffSign) )		return v2;
    else if ( VAL_is_le(diffSign) )	return v1;
    else				return VAL_BOTTOM;

}  /* end of val_max() */





/**************************************************************************** 
 *  -- val_gcd
 *
 *     Find greatest common divisor of two values.
 ****************************************************************************/

ValNumber val_gcd(Values Vp,
		  ValNumber v1,
		  ValNumber v2)
{
    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;
    if (v1 == VAL_TOP) v1 = VAL_ZERO;
    if (v2 == VAL_TOP) v2 = VAL_ZERO;

    if (v1 == v2) return v1;

    int coeff1, coeff2, coeff, added;

    if (ve_type((*Vp)[v1]) == VAL_CONST) 
    {
	coeff1 = ve_const((*Vp)[v1]);
    }
    else
    {
	val_const_parts(*Vp, v1, coeff, added);
	coeff1 = Gcd(coeff, added);
    }
    if (ve_type((*Vp)[v2]) == VAL_CONST) 
    {
	coeff2 = ve_const((*Vp)[v2]);
    }
    else
    {
	val_const_parts(*Vp, v2, coeff, added);
	coeff2 = Gcd(coeff, added);
    }
    return val_lookup_const(*Vp, ve_expType((*Vp)[v1]),
			    Gcd(coeff1, coeff2));
}




/**************************************************************************** 
 *  -- val_lcm
 *
 *     Least common multiple of two value numbers.
 ****************************************************************************/

ValNumber val_lcm(Values Vp,
		  ValNumber v1,
		  ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    if (v1 == v2) return v1;

    if ((ve_type((*Vp)[v1]) != VAL_CONST) ||
	(ve_type((*Vp)[v2]) != VAL_CONST))
    {
	/*
	 *  Close enough in symbolic case?
	 */
	return val_binary(Vp, VAL_OP_TIMES, v1, v2);
    }

    return val_lookup_const(*Vp, ve_expType((*Vp)[v1]),
			    Lcm(ve_const((*Vp)[v1]),
				ve_const((*Vp)[v2])));
}


ValNumber val_div_ceil(Values Vp,
		       ValNumber v1,
		       ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    if (ve_type((*Vp)[v2]) != VAL_CONST) return VAL_BOTTOM;

    int c2 = ve_const((*Vp)[v2]);

    int result;

    if (ve_type((*Vp)[v1]) != VAL_CONST)
    {
	/*  int coeff, added;
	 *  val_const_parts(*Vp, v1, coeff, added);
	 *
	 *  if ((coeff % c2) == 0) result = EMod(added, c2);
	 *  else
	 */
	return VAL_BOTTOM;
    }
    else
    {
	int c1 = ve_const((*Vp)[v1]);
	result = c1/c2;
	if (c1 % c2) result++;
	return val_lookup_const((*Vp), ve_expType((*Vp)[v1]), result);
    }
}

ValNumber val_div_floor(Values Vp,
			ValNumber v1,
			ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    if (ve_type((*Vp)[v2]) != VAL_CONST) return VAL_BOTTOM;

    int c2 = ve_const((*Vp)[v2]);

    int result;

    if (ve_type((*Vp)[v1]) != VAL_CONST)
    {
	/*  int coeff, added;
	 *  val_const_parts(*Vp, v1, coeff, added);
	 *
	 *  if ((coeff % c2) == 0) result = EMod(added, c2);
	 *  else
	 */
	return VAL_BOTTOM;
    }
    else
    {
	int c1 = ve_const((*Vp)[v1]);
	result = c1/c2;
	return val_lookup_const((*Vp), ve_expType((*Vp)[v1]), result);
    }
}


/**************************************************************************** 
 *  -- val_emod
 *
 *     Take Euclidean mod of two value numbers.
 ****************************************************************************/

ValNumber val_emod(Values Vp,
		   ValNumber v1,
		   ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    if (ve_type((*Vp)[v2]) != VAL_CONST) return VAL_BOTTOM;

    int c2 = ve_const((*Vp)[v2]);

    int result;

    if (ve_type((*Vp)[v1]) != VAL_CONST)
    {
	int coeff, added;
	val_const_parts(*Vp, v1, coeff, added);

	if ((coeff % c2) == 0) result = EMod(added, c2);
	else		       return VAL_BOTTOM;
    }
    else
	result = EMod(ve_const((*Vp)[v1]), c2);

    return val_lookup_const((*Vp), ve_expType((*Vp)[v1]), result);
}



/**************************************************************************** 
 *  -- val_mod
 *
 *     Take remainder of v1/v2.
 ****************************************************************************/

ValNumber val_mod(Values Vp,
		   ValNumber v1,
		   ValNumber v2)
{
    if ((v1 == VAL_TOP) || (v2 == VAL_TOP))       return VAL_TOP;

    if ((v1 == VAL_BOTTOM) || (v2 == VAL_BOTTOM)) return VAL_BOTTOM;

    if (ve_type((*Vp)[v2]) != VAL_CONST) return VAL_BOTTOM;

    int c2 = ve_const((*Vp)[v2]);

    int result;

    if (ve_type((*Vp)[v1]) != VAL_CONST)
    {
	int coeff, added;
	val_const_parts(*Vp, v1, coeff, added);

	if ((coeff % c2) == 0) result = added % c2;
	else		       return VAL_BOTTOM;
    }
    else result = ve_const((*Vp)[v1]) % c2;

    return val_lookup_const((*Vp), ve_expType((*Vp)[v1]), result);
}



/*
 *  Returns base after subtracting constant part and dividing by largest
 *  constant divisor of symbolic part.  Added constant and constant coeff
 *  are set.
 */
ValNumber val_get_base(ValTable &V, ValNumber vn, int &coeff, int &added)
{
    if ((ve_type(V[vn]) == VAL_OP) && (ve_opType(V[vn]) == VAL_OP_PLUS))
    {
        added = ve_const(V[ve_left(V[vn])]);
        vn = ve_right(V[vn]);
    }
    else added = 0;

    if ((ve_type(V[vn]) == VAL_OP) && (ve_opType(V[vn]) == VAL_OP_TIMES))
    {
        coeff = ve_const(V[ve_left(V[vn])]);
        vn = ve_right(V[vn]);
    }
    else coeff = 1;

    return vn;
}
/*
 *  Returns constant added part and largest constant divisor of symbolic
 *  part, without necessarily building the base value number.
 */
void val_const_parts(ValTable &V, ValNumber vn, int &coeff, int &added)
{
    if ((ve_type(V[vn]) == VAL_OP) && (ve_opType(V[vn]) == VAL_OP_PLUS))
    {
        added = ve_const(V[ve_left(V[vn])]);
        vn = ve_right(V[vn]);
    }
    else added = 0;

    if ((ve_type(V[vn]) == VAL_OP) && (ve_opType(V[vn]) == VAL_OP_TIMES))
    {
        coeff = ve_const(V[ve_left(V[vn])]);
        vn = ve_right(V[vn]);
    }
    else coeff = 1;
}
