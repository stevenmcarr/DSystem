/* $Id: val_range.C,v 2.9 1997/03/11 14:36:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 * -- val_range.c
 * 
 *          This file contains subprograms that performs range analysis
 *        on regular sections with bound and Step.
 *        Each range entry will occupy one value table entry.
 *       
 *   By: Po-Jen Yang                Last Modified: June 1991
 ****************************************************************************/

#include <libs/moduleAnalysis/valNum/val.i>
#include <stdio.h>

static void trim_hi(ValTable &V, ValEntry * valNode);

/****************************************************************************
 **			   PUBLIC FUNCTIONS                                **
 ****************************************************************************/

/*---------------------------------------------------------------------------
 * -- val_standardize
 * 
 *    Transform the range node into a standard form with
 *    the range running in increasing order and bounds
 *    trimmed to be element of the set whenever possible.
 *
 *    Precondition: * valNode->type = VAL_RANGE (==>  valNode != TOP or BOTTOM)
 *                  * For better result, entries already in the value table 
 *                    must not have degenerate ranges. (like a constant or 
 *                    empty)
 *                  *  valNode must NOT be a pointer to the value table
 *
 *    Postcondition: 
 *            * A valid value number is returned when the result is 
 *              a known value number.  It is up to the caller to 
 *              free the value numbers in valNode when a new value number
 *              is to be used.
 *            * flipped, if not NULL, will contain the information
 *              whether the bounds has been flipped
 *                   
 *---------------------------------------------------------------------------*/

ValNumber val_standardize(ValTable &V,
			  ValEntry * valNode)
{
    assert(ve_type(*valNode) == VAL_RANGE);

    if ((ve_lo(*valNode) == VAL_BOTTOM) && (ve_lo(*valNode) == VAL_BOTTOM) &&
	((ve_step(*valNode) == VAL_BOTTOM) || (ve_step(*valNode) == VAL_ONE)))
    {
	return VAL_BOTTOM;
    }

    ValNumber diff;		// difference between upper and lower bound
    ValNumber newAlign;
    ValSign signDiff, signStep; // sign of bound difference and of step

    trim_hi(V,valNode);		// hi bound may be misaligned

    if ((ve_hi(*valNode) == ve_lo(*valNode)) && 
	(ve_type(V[ve_lo(*valNode)]) != VAL_RANGE))
    {
	return(ve_lo(*valNode));     /* degenerate to a non-range symbolic */
    }
  
    diff = val_binary(&V, VAL_OP_MINUS, ve_hi(*valNode), ve_lo(*valNode));
    signDiff = val_sign(&V,diff); 

    //  If the bounds are the same, the value number should be
    //  the same. Thus first "if" should catch (signDiff == VAL_SIGN_ZERO)
    //  already, but just to be safe ...  
    //  Following test could be removed later.
    //
    if (signDiff == VAL_SIGN_ZERO) /* degenerate to initial bound */
	return(ve_lo(*valNode));  

    //
    //  Note: From here on, (hi - lo) != 0 
    //
    signStep = val_sign(&V,ve_step(*valNode));
    switch(signStep)
    {
      case VAL_SIGN_NONPOS: 
      case VAL_SIGN_NEG:
	/* 
	 * assume signStep is really VAL_SIGN_NEG 
	 * since step of zero is impossible in Fortran 
	 */	
	if (signDiff == VAL_SIGN_POS)         /* empty range */
	    return( VAL_TOP );
	else if (signDiff == VAL_SIGN_NONNEG)
	{
	    /*
	     *  The worst case would seem to be degenerating to starting bound,
	     *  but that may be too definite -- the range could just be empty.
	     */
	    ve_simple(*valNode) = (ValField) false;
	    ve_cRange(*valNode) = ve_cRange(V[ve_lo(*valNode)]);
	    return VAL_NIL;
	}

	/* 
	 * else  (signDiff == NEG, NONPOS, or UNKNOWN)
	 *       check for degenerating to starting bound ==> (diff - step > 0)
	 *       if not, simply flip "upper" and "lower" bound
	 *	 (starting bound is higher than ending)
	 */
	if ( val_compare(&V,diff, ve_step(*valNode)) == VAL_CMP_GT )
	    return(ve_lo(*valNode));     

	val_swap(&ve_lo(*valNode), &ve_hi(*valNode));
	ve_step(*valNode) = val_unary(&V,VAL_OP_MINUS,ve_step(*valNode));
	ve_align(*valNode) = val_unary(&V,VAL_OP_MINUS,ve_align(*valNode));
	newAlign = val_emod(&V, ve_align(*valNode), ve_step(*valNode));
	//
	//  Leave alignment symbolic if not constant.
	//
	if (ve_type(V[newAlign]) == VAL_CONST)
	{
	    ve_align(*valNode) = newAlign;
	}
	ve_simple(*valNode) = (ValField) true;
	return(VAL_NIL);   /* valNode is now standardized */

	
    case VAL_SIGN_NONNEG:
    case VAL_SIGN_POS:
	/* 
	 * assume signStep is really VAL_SIGN_POS 
	 * since step of zero is impossible in Fortran 
	 */	
	if (signDiff == VAL_SIGN_NEG)         /* empty range */
	    return( VAL_TOP );

	else if (signDiff == VAL_SIGN_NONPOS) /* at worst a constant */
	{
	    /*
	     *  The worst case would seem to be degenerating to lower bound,
	     *  but that may be too definite -- the range could just be empty.
	     */
	    ve_simple(*valNode) = (ValField) false;
	    ve_cRange(*valNode) = ve_cRange(V[ve_lo(*valNode)]);
	    return VAL_NIL;
	}

	if ( val_compare(&V, diff, ve_step(*valNode)) == VAL_CMP_LT )
	    return(ve_lo(*valNode));     /* degenerate to lower bound */

	// else
	newAlign = val_emod(&V, ve_align(*valNode), ve_step(*valNode));
	//
	//  Leave alignment symbolic if not constant.
	//
	if (ve_type(V[newAlign]) == VAL_CONST)
	{
	    ve_align(*valNode) = newAlign;
	}
	ve_simple(*valNode) = (ValField) true;

	return(VAL_NIL);	/* valNode is now standardized */
  	
    case VAL_SIGN_UNKNOWN:	/* sign of step is unknown */

	if (signDiff == VAL_SIGN_UNKNOWN)
	{
	    ve_simple(*valNode) = (ValField) false;
	    return(VAL_NIL);            /* Not standardizable */
	}
	
	/* 
	 * else we know the sign of (hi - lo)
	 */	
	if ( (signDiff == VAL_SIGN_NEG) || (signDiff == VAL_SIGN_NONPOS) )
	{
	    /* non-positive ==> upper <= lower bound */
	    val_swap(&ve_lo(*valNode), &ve_hi(*valNode));

	    /*
	     *  negate the step, but don't bother trying to 
	     *  fix alignment
	     */
	    ve_step(*valNode) = val_unary(&V, VAL_OP_MINUS,
	                                  ve_step(*valNode)); 
	}

	ve_simple(*valNode) = (ValField) true;
	return( VAL_NIL );
	
    case VAL_SIGN_ZERO:  /* zero step is impossible in Fortran */
    default:
	die_with_message("Error in val_standardize: step == zero \n\n");
	return(VAL_NIL);
    }  /* end of switch on signStep */

    /* Control should never reach here */
}  /* end of function val_standardize()  */






/****************************************************************************
 **			   PRIVATE FUNCTIONS                               **
 ****************************************************************************/


/*---------------------------------------------------------------------------
 * -- trim_hi
 *
 *      Trim upper bound only; lower bound should already be aligned.
 *      A range is trimmable the following can be evaluated 
 *            newHi = hi - ( hi - align ) % step
 *
 *      Precondition: valNode must be a RANGE node.
 *                    valNode must NOT be a pointer to the value table
 *--------------------------------------------------------------------------*/

static void trim_hi(ValTable &V,
	    	    ValEntry * valNode)
{
    ValNumber step = ve_step(*valNode);

    if (ve_type(V[step]) != VAL_CONST)
    {
	return;
    }
    ValNumber diff, oldHi, remain;

    step = ve_step(*valNode);
    oldHi = ve_hi(*valNode);
    diff = val_binary(&V,VAL_OP_MINUS, oldHi, ve_align(*valNode));
    
    if (ve_type(V[diff]) == VAL_CONST)
    {
	remain = val_binary(&V, VAL_OP_MOD, diff, step);
	ve_hi(*valNode) = val_binary(&V, VAL_OP_MINUS,
				     oldHi, remain);
    }
}

//  val_get_const_range
//
//	return value	means:
//
//	VAL_TOP		empty (not executed)
//	VAL_BOTTOM	no constant bounds proven
//	type VAL_CONST	value is constant
//	type VAL_RANGE	some constant bounds info proven:
//			lo, hi, step, align are VAL_BOTTOM or constant
//			step and align are linked -- both must be
//				constant or both VAL_BOTTOM, also
//				constant align must be in [0,step).
//
ValNumber val_get_const_range(Values Vp, ValNumber v)
{
    int i;

    assert(v > VAL_NIL);

    ValEntry &ve = (*Vp)[v];

    if (ve_cRange(ve) != VAL_NIL)
    {
	return ve_cRange((*Vp)[v]);
    }

    ValNumber cRange = VAL_BOTTOM;

    switch(ve_type(ve))
    {
      case VAL_TOP_TYPE:
	assert(v == VAL_TOP);
	cRange = VAL_TOP;
	break;

      case VAL_BOT_TYPE:
	assert(v == VAL_BOTTOM);
	cRange = VAL_BOTTOM;
	break;

      case VAL_CONST:
	cRange = v;
	break;

      case VAL_RANGE:
	ValNumber lo, hi, align, step;

	if (!ve_simple(ve)) lo = hi = VAL_BOTTOM;
	else
	{
	    lo = val_get_lo(Vp, ve_lo(ve));		// constant or BOTTOM
	    hi = val_get_hi(Vp, ve_hi(ve));		// constant or BOTTOM
	}
	align = val_get_const_range(Vp, ve_align(ve));	// const, BOT, or range
	step  = val_get_const_range(Vp, ve_step(ve));	// const, BOT, or range
	if ((lo == ve_lo(ve)) &&
	    (hi == ve_hi(ve)) &&
	    (align == ve_align(ve)) &&
	    (step  == ve_step(ve)))
	{
	    //  already a constant range
	    //
	    cRange = v;
	    break;
	}

	//  must be a standardized but non-constant range,
	//  such as for an IVAR

	//  get something that always divides step
	//
	if (ve_type((*Vp)[step]) == VAL_RANGE)
	{
	    step = val_gcd(Vp, ve_step((*Vp)[step]), ve_align((*Vp)[step]));
	}
	if ((ve_type((*Vp)[step]) != VAL_CONST) ||
	    (ve_const((*Vp)[step]) <= 1))
	{
	    /*
	     *  0 or negative is error -- should be normalized away 
	     *  to constant or forward range by now.  1 is meaningless.
	     */
	    step = align = VAL_BOTTOM;
	}
	else if (align == VAL_BOTTOM)
	{
	    step = VAL_BOTTOM;
	}
	else if (ve_type((*Vp)[align]) == VAL_RANGE)
	{
	    //  other possibilities were constant or BOTTOM
	    //
	    if (val_emod(Vp, ve_step((*Vp)[align]), step) == 0)
	    {
		//  step divides step of alignment --
		//      align is range with A%S == A
		//      step is constant R
		//      R|S so new alignment is A%R
		//
		align = val_emod(Vp, ve_align((*Vp)[align]), step);
	    }
	    else
	    {
		step = align = VAL_BOTTOM;
	    }
	}
	else
	{
	    assert(ve_type((*Vp)[align]) == VAL_CONST);
	    
	    align = val_emod(Vp, align, step);
	}
	if ((lo == VAL_BOTTOM) && (hi == VAL_BOTTOM) &&
	    ((step == VAL_BOTTOM) || (step == VAL_ONE)))
	{
	    cRange = VAL_BOTTOM;
	}
	else
	{
	    // assert(ve_const((*Vp)[step]) > 1);	// not negative, 0 or 1
	    cRange = val_lookup_range((*Vp), lo, hi, align, step);
	}
	break;

      case VAL_IVAR:
	cRange = val_get_const_range(Vp, ve_bounds(ve));
	break;

      case VAL_OP:
	switch(ve_opType(ve))
	{
	  case VAL_OP_PLUS:	// works for any simple ranges
	  case VAL_OP_TIMES:	// min-max(L1*U1,L1*U2,L2*U1,L2*U2)
	  case VAL_OP_MAX:	// simple
	  case VAL_OP_MIN:	// simple
	    cRange = val_binary(Vp, ve_opType(ve),
				val_get_const_range(Vp, ve_kid(ve,0)),
				val_get_const_range(Vp, ve_kid(ve,1)));

	    for (i = 2; i < ve_arity(ve); i++)
	    {
		cRange = val_binary(Vp, ve_opType(ve), cRange,
				    val_get_const_range(Vp, ve_kid(ve,i)));
	    }
	    break;

	  case VAL_OP_INT:
	  case VAL_OP_NINT:
	  case VAL_OP_REAL:
	  case VAL_OP_DOUBLE:
	    //
	    //  These are just type conversions
	    //
	    cRange = val_get_const_range(Vp, ve_kid(ve,0));
	    break;

	  case VAL_OP_DIVIDE:
	  case VAL_OP_ABS:
	  case VAL_OP_MOD:
	    //  should be smarter than this
	  default:
	    cRange = VAL_BOTTOM;
	    break;
	}
	break;

      case VAL_PHI:
      case VAL_GAMMA:
	cRange = VAL_TOP;
	for (i = 0; i < ve_arity(ve); i++)
	{
	    cRange = val_merge(Vp, cRange, 
			       val_get_const_range(Vp, ve_kid(ve, i)));
	}
	break;

      case VAL_ETA:
	cRange = val_get_const_range(Vp, ve_final(ve));
	break;

      case VAL_ARRAY:
	cRange = val_get_const_range(Vp, ve_state(ve));
	if (ve_rhs(ve) != VAL_NIL)
	{
	    cRange = val_merge(Vp, cRange,
			       val_get_const_range(Vp, ve_rhs(ve)));
	}
	break;

      default:
	break;
    }
    ValType cType = ve_type((*Vp)[cRange]);
    assert((cType == VAL_CONST) || (cType == VAL_RANGE) || 
	   (cRange == VAL_BOTTOM) || (cRange == VAL_TOP));

    ve_cRange((*Vp)[cRange]) = cRange;	// this is how we check for constRange
    return ve_cRange(ve) = cRange;	// set and return
}

//  val_get_{lo,hi,step,align}
//	return constant bounds information
//
ValNumber val_get_lo(Values Vp, ValNumber v)
{
    if (!val_is_value(v)) return v;

    ValNumber range = val_get_const_range(Vp, v);

    if ((!val_is_value(range)) || (ve_type((*Vp)[range]) == VAL_CONST))
    {
	return range;
    }
    else
    {
	assert(ve_type((*Vp)[range]) == VAL_RANGE);
	return ve_lo((*Vp)[range]);
    }
}
ValNumber val_get_hi(Values Vp, ValNumber v)
{
    if (!val_is_value(v)) return v;

    ValNumber range = val_get_const_range(Vp, v);

    if ((!val_is_value(range)) || (ve_type((*Vp)[range]) == VAL_CONST))
    {
	return range;
    }
    else
    {
	assert(ve_type((*Vp)[range]) == VAL_RANGE);
	return ve_hi((*Vp)[range]);
    }
}
ValNumber val_get_step(Values Vp, ValNumber v)
{
    if (!val_is_value(v)) return v;

    ValNumber range = val_get_const_range(Vp, v);

    if (!val_is_value(range)) return range;

    if (ve_type((*Vp)[range]) == VAL_CONST) return VAL_TOP;

    assert(ve_type((*Vp)[range]) == VAL_RANGE);

    return ve_step((*Vp)[range]);
}
ValNumber val_get_align(Values Vp, ValNumber v)
{
    if (!val_is_value(v)) return v;

    ValNumber range = val_get_const_range(Vp, v);

    if ((!val_is_value(range)) || (ve_type((*Vp)[range]) == VAL_CONST))
    {
	return range;
    }
    else
    {
	assert(ve_type((*Vp)[range]) == VAL_RANGE);
	return ve_align((*Vp)[range]);
    }
}


//  val_merge
//	Returns a constant range that contains every value in the constant
//	range of either input value.
//
ValNumber val_merge(Values Vp, ValNumber v1, ValNumber v2)
{
    if ((v2 == VAL_TOP) || (v1 == VAL_BOTTOM)) return v1;
    if ((v1 == VAL_TOP) || (v2 == VAL_BOTTOM)) return v2;
    if (v1 == v2) return v1;

    assert(val_is_value(v1) && val_is_value(v2));

    if (ve_type((*Vp)[v1]) == TYPE_LOGICAL) return VAL_BOTTOM;

    ValNumber L,U,A,S;

    L = val_min(Vp, val_get_lo(Vp, v1), val_get_lo(Vp, v2));
    U = val_max(Vp, val_get_hi(Vp, v1), val_get_hi(Vp, v2));

    val_merge_steps(*Vp, S, A,
		    val_get_step(Vp, v1), val_get_step(Vp, v2),
		    val_get_align(Vp, v1), val_get_align(Vp, v2));

    return val_lookup_range(*Vp, L, U, A, S);
}

//  val_merge_steps
//	Computes new step and alignment based on old ones --
//	results are returns as side effects to newS and newA.
//
void val_merge_steps(ValTable &V, ValNumber &newS, ValNumber &newA,
		     ValNumber s1, ValNumber s2, ValNumber a1, ValNumber a2)
{
    newS = val_gcd(&V, s1, s2);

    ValNumber diff = val_binary(&V, VAL_OP_MINUS, a1, a2);

    newS = val_gcd(&V, newS, diff);
    newA = val_emod(&V, a1, newS);

    return;
}
