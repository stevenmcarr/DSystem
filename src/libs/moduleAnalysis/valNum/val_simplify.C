/* $Id: val_simplify.C,v 2.13 2001/10/12 19:29:02 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 * -- val_simplify.c
 *
 *          This file contains functions that simplfies a value node
 *
 *          By: Tim Mullin          
 *              intrinsic functions stuff added by Po-Jen Yang July 1991
 ****************************************************************************/

#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/support/numerical/ExtendedGCD.h>
#undef is_open
#include <iostream.h>

extern char *D_sym_simplify;

static const unsigned int MAX_SCORE = ~0;

static unsigned long score(ValTable &V, ValNumber vn);

static ValNumber simplify_plus(ValTable &V, ValEntry *value);
static ValNumber simplify_times(ValTable &V, ValEntry *value);
static ValNumber combine(ValTable &V, ValOpType op,
			 ValNumber v1, ValNumber v2);
static ValNumber sorted_merge(ValTable &V, ValOpType op, 
			      ValNumber v1, ValNumber v2);

static ValNumber range_simplify(ValTable &V, ValEntry *value);

static ValNumber combine_constants(ValTable &V,
				   ValOpType opType,
				   ExpType expType,
				   int L,
				   int R);
static void val_order(ValTable &V,
		      ValEntry *ve);

static ValNumber val_simplify_unary(ValTable &V,
				    ValEntry *value);

static inline Boolean is_sum(ValTable &V, ValNumber vn)
{
    return BOOL((ve_type(V[vn]) == VAL_OP) &&
		(ve_opType(V[vn]) == VAL_OP_PLUS));
}
static inline Boolean is_term(ValTable &V, ValNumber vn)
{
    return BOOL((ve_type(V[vn]) == VAL_OP) &&
		(ve_opType(V[vn]) == VAL_OP_TIMES));
}
static inline int extract_coeff(ValTable &V, ValNumber &vn)
{
    if (is_term(V, vn) && ve_type(V[ve_left(V[vn])]) == VAL_CONST)
    {
	int coeff = ve_const(V[ve_left(V[vn])]);
	vn = ve_right(V[vn]);
	return coeff;
    }
    else
	return 1;
}
static inline int extract_added(ValTable &V, ValNumber &vn)
{
    if (is_sum(V, vn) && ve_type(V[ve_left(V[vn])]) == VAL_CONST)
    {
	int added = ve_const(V[ve_left(V[vn])]);
	vn = ve_right(V[vn]);
	return added;
    }
    else
	return 0;
}

/****************************************************************************
 **			   PUBLIC  FUNCTION                                **
 ****************************************************************************/

/******************************************************************/
/* val_simplify takes an item to be placed on the value table and */
/* simplifies it.  It distributes multiplication over addition,   */
/* sorts, combines terms, and performs a few other obvious 	  */
/* rewrites.  Constants stay at the top left of products and 	  */
/* sums.  Simplification of (nonconstant * sum) not yet supported.*/
/*                                                                */
/*                                           paco, Mar 1993       */
/*                                                                */
/*    Precondition: value node must not be in value table         */
/*                                                                */
/* NOTE:  returning VAL_NIL means that only that    	          */
/*        simplification has not produced a value already in the  */
/*        table.	                                          */
/*                                                                */
/******************************************************************/
ValNumber val_simplify(ValTable &V,
		       ValEntry *value)
{
    ValNumber	R, L;
    ValNumber   temp;

    if (ve_type(*value) != VAL_OP)
	die_with_message("val_simplify: attempt to simplify non-operator\n");

    if (ve_arity(*value) == 1) return val_simplify_unary(V, value);

    if (ve_arity(*value) != 2) return VAL_NIL; // not simplified

    L = ve_left(*value);
    R = ve_right(*value);

    if ((L == VAL_TOP) || (R == VAL_TOP)) return VAL_TOP;	// unexecutable

    if ((L == VAL_BOTTOM)||(R == VAL_BOTTOM)) return VAL_BOTTOM;// bogosity

    /*
     *  Only handle numeric and logical values.
     */
    if ((ve_expType(*value) != TYPE_INTEGER) &&
	(ve_expType(*value) != TYPE_REAL) &&
	(ve_expType(*value) != TYPE_DOUBLE_PRECISION) &&
	(ve_expType(*value) != TYPE_LOGICAL))
    {
	return VAL_NIL;		// no change, not entered into table yet
    }

    /*
     * If binary tree op of two constants then combine and return the result
     */
    if ((ve_type(V[L]) == VAL_CONST) &&
	(ve_type(V[R]) == VAL_CONST))
    {
	/*
	 *  Combination may fail for reasons like overflow;
	 *  then VAL_NIL is properly returned.
	 */
        return combine_constants(V, ve_opType(*value), 
				 ve_expType(*value),
				 ve_const(V[L]),
				 ve_const(V[R]));
    }

    if ((ve_cRange(V[L]) == L) && (ve_cRange(V[R]) == R))
    {
	//  We're dealing with constants and ranges of constants
	//
	return range_simplify(V, value);
    }

    /*
     *  Allow simplification to be turned off 
     */
    if (strcmp(D_sym_simplify, "on")) return VAL_NIL;

    /*
     *  Handle symbolic operations only for integer and logical types
     */
    if ((ve_expType(*value) != TYPE_INTEGER) && 
	(ve_expType(*value) != TYPE_LOGICAL))
    {
	return VAL_NIL;		// no change, not entered into table yet
    }

    switch (ve_opType(*value))
    {
      case VAL_OP_PLUS:
	return simplify_plus(V, value);
	break;
	
      case VAL_OP_MINUS:
	if (L == R) return VAL_ZERO;

	ve_opType(*value) = VAL_OP_PLUS;
        ve_right(*value) = val_binary(&V, VAL_OP_TIMES,
				      VAL_M_ONE, ve_right(*value));
	return simplify_plus(V, value);
        break;
	
      case VAL_OP_TIMES:
	return simplify_times(V, value);
	break;
	
      case VAL_OP_DIVIDE:
	if (ve_type(V[L]) == VAL_CONST)
	{
	    if (ve_const(V[L]) == 0) return VAL_ZERO;
	    int coeff = extract_coeff(V, R);

	    L = combine_constants(V, VAL_OP_DIVIDE, 
				  ve_expType(*value),
				  ve_const(V[L]), coeff);

	    if (L != VAL_NIL)
	    {
		ve_left(*value)  = L;
		ve_right(*value) = R;	// possibly changed by extract_coeff
	    }
	    return VAL_NIL;
	}
	if (ve_type(V[R]) == VAL_CONST)
        {
	    int coeff = extract_coeff(V,L);

	    if ((coeff % ve_const(V[R])) != 0) return VAL_NIL;

	    R = combine_constants(V, VAL_OP_DIVIDE,
				  ve_expType(*value),
				  coeff, ve_const(V[R]));

	    if (R != VAL_NIL)	// R is new coeff of numerator
	    {
		ve_left(*value)   = R;
		ve_right(*value)  = L;	// possibly changed by extract_coeff
		ve_opType(*value) = VAL_OP_TIMES;

		return simplify_times(V, value);
	    }
	    return VAL_NIL;
	}
	// neither is constant... don't bother for now

	return VAL_NIL;
        break;
	
      case VAL_OP_AND:
	val_order(V, value);
        if (ve_left(*value) == VAL_TRUE)  return ve_right(*value);
	if (ve_left(*value) == VAL_FALSE) return VAL_FALSE;
	break;
	
      case VAL_OP_OR:
	val_order(V, value);
        if (ve_left(*value) == VAL_TRUE)  return VAL_TRUE;
	if (ve_left(*value) == VAL_FALSE) return ve_right(*value);
        break;
	
      case VAL_OP_LE:
	ve_opType(*value) = VAL_OP_GE;
	val_swap(&ve_left(*value), &ve_right(*value));
	/*
	 *  fall through to .GE.
	 */
	
      case VAL_OP_GE:
	if ((ve_expType(V[ve_left(*value)]) != TYPE_INTEGER) ||
	    (ve_expType(V[ve_right(*value)]) != TYPE_INTEGER)) break;
	ve_left(*value) = 
	    val_binary(&V,VAL_OP_MINUS, ve_left(*value), ve_right(*value));
	if (ve_type(V[ve_left(*value)]) == VAL_CONST)
	{
	    if (ve_const(V[ve_left(*value)]) >= 0) return VAL_TRUE;
	}
	ve_right(*value) = VAL_ZERO;
        break;
	
      case VAL_OP_LT:
	ve_opType(*value) = VAL_OP_GT;
	val_swap(&ve_left(*value), &ve_right(*value));
	/*
	 *  fall through to .GT.
	 */
	
      case VAL_OP_GT:
	if ((ve_expType(V[ve_left(*value)]) != TYPE_INTEGER) ||
	    (ve_expType(V[ve_right(*value)]) != TYPE_INTEGER)) break;
	ve_left(*value) =
	    val_binary(&V,VAL_OP_MINUS, ve_left(*value), ve_right(*value));
	if (ve_type(V[ve_left(*value)]) == VAL_CONST)
	{
	    if (ve_const(V[ve_left(*value)]) > 0) return VAL_TRUE;
	}
	ve_right(*value) = VAL_ZERO;
        break;
	
      case VAL_OP_NE:
	ve_opType(*value) = VAL_OP_EQ;
	return val_unary(&V, VAL_OP_NOT, val_binary(&V, VAL_OP_EQ,
						    ve_left(*value),
						    ve_right(*value)));
        break;
	
      case VAL_OP_EQ:
	if (L == R) return VAL_TRUE;
	val_order(V, value);
	L = ve_left(*value);
	R = ve_right(*value);
	if ((ve_expType(V[L]) != TYPE_INTEGER) ||
	    (ve_expType(V[R]) != TYPE_INTEGER)) break;
	ve_right(*value) = 
	    val_binary(&V,VAL_OP_MINUS, L, R);
	ve_left(*value) = VAL_ZERO;
        break;

      case VAL_OP_MIN:
	temp = val_min(&V, L, R);
	if (temp != VAL_BOTTOM) return temp;
	break;

      case VAL_OP_MAX:
	temp = val_max(&V, L, R);
	if (temp != VAL_BOTTOM) return temp;
	break;

      case VAL_OP_MOD:
	if (L == R) return VAL_ZERO;
	break;

      default:
        break;
    }
    return VAL_NIL;

} /* end of function val_simplify_tree_op() */



/***************************************************************/
/* combine_constants() evaluates an operation on two           */
/* constants.  Type correctness is assumed.  Return true on    */
/* success.						       */
/***************************************************************/
static ValNumber combine_constants(ValTable &V,
				   ValOpType opType,
				   ExpType expType,
				   int L,
				   int R)
{
    int result;
    int success = false;

    /*
     *  Pray against overflow!
     */
    switch (opType)
    {
      case VAL_OP_PLUS:
        result  = L + R;
	success = true;
	break;

      case VAL_OP_MINUS:
	result  = L - R;
	success = true;
	break;

      case VAL_OP_TIMES:
	result  = L * R;
	success = true;
	break;

      case VAL_OP_MOD:
	result  = L % R;
	success = true;
	break;

      case VAL_OP_DIVIDE:
	if ((R) == 0)
	    cerr << "Divide by 0: " << L << "/0\n";
	else
	{
	    if ((expType == TYPE_INTEGER) || ((L % R) == 0))
	    {
		result  = L / R;
		success = true;
	    }
	}
	break;

      case VAL_OP_GE:
	result  = BOOL(L >= R);
	success = true;
	break;

      case VAL_OP_LE:
	result  = BOOL(L <= R);
	success = true;
	break;

      case VAL_OP_GT:
	result  = BOOL(L > R);
	success = true;
	break;

      case VAL_OP_LT:
	result  = BOOL(L < R);
	success = true;
	break;

      case VAL_OP_NE:
	result  = BOOL(L != R);
	success = true;
	break;

      case VAL_OP_EQ:
	result  = BOOL(L == R);
	success = true;
	break;

      case VAL_OP_AND:
	result  = BOOL(L && R);

      case VAL_OP_OR:
	result  = BOOL(L || R);
	success = true;
	break;

      default:
	break;
    }
    if (success)
	return val_lookup_const(V, expType, result);
    else
	return VAL_NIL;	// not simplified

} /* end combine_constants */


/*
 *  val_order:  take a commutative binary operator and order its operands
 *		constant on left, 
 */
static void val_order(ValTable &V, ValEntry *ve)
{
    int opType = ve_opType(*ve);

    switch(opType)
    {
      case VAL_OP_PLUS:
      case VAL_OP_TIMES:
      case VAL_OP_AND:
      case VAL_OP_OR:
	break;
      default:
	return;
    }
    ValEntry *L = &(V[ve_left(*ve)]);
    ValEntry *R = &(V[ve_right(*ve)]);

    /*
     *  Any of these tests should result in constant being on the left
     */

    if (ve_type(*L) == VAL_CONST) return;	// want constant on left
    if (ve_type(*R) == VAL_CONST)
    {
	val_swap(&ve_left(*ve), &ve_right(*ve));
	return;
    }

    if (ve_expType(*ve) == TYPE_INTEGER)	// PLUS or TIMES
    {
	if (((ve_type(*R) != VAL_OP) ||
	     (ve_opType(*R) != opType)) &&
	    ((ve_type(*L) == VAL_OP) &&
	     (ve_opType(*L) == opType)))
	{
	    /*
	     *  is in form p*x or s+x, want x*p or x+s
	     */
	    val_swap(&ve_left(*ve), &ve_right(*ve));
	    return;
	}
    }

    /*
     *  In absence of above decisions, order operands by value number
     *
     *  Higher score goes on left.
     */
    if (score(V, ve_left(*ve)) < score(V, ve_right(*ve)))
    {
	val_swap(&ve_left(*ve), &ve_right(*ve));
    }
}

static ValNumber val_simplify_unary(ValTable &V,
				    ValEntry *value)
{
    ValNumber L = ve_left(*value);

    switch(ve_opType(*value))
    {
      case VAL_OP_MINUS:
	return val_binary(&V, VAL_OP_TIMES, VAL_M_ONE, L);

      case VAL_OP_NOT:
	if (L == VAL_TRUE)  return VAL_FALSE;
	if (L == VAL_FALSE) return VAL_TRUE;

        if (ve_type(V[L]) == VAL_OP)
        {
            switch(ve_opType(V[L]))
            {
	      case VAL_OP_NOT: return ve_left(V[L]);

              case VAL_OP_GE: return val_binary(&V, VAL_OP_LT,
						    ve_left(V[L]),
						    ve_right(V[L]));
		
              case VAL_OP_LE: return val_binary(&V, VAL_OP_GT,
						    ve_left(V[L]),
						    ve_right(V[L]));

              case VAL_OP_GT: return val_binary(&V, VAL_OP_LE,
						    ve_left(V[L]),
						    ve_right(V[L]));
		
              case VAL_OP_LT: return val_binary(&V, VAL_OP_GE,
						    ve_left(V[L]),
						    ve_right(V[L]));
	      /*
	       *  (L .NE. R) done as .NOT.(L .EQ. R)
	       */

              default: break;
            }
        }
	break;

      case VAL_OP_ABS:
      {
	  ValSign sign = val_sign(&V, L);

	  if (VAL_is_nonneg(sign)) return L;
	  if (VAL_is_nonpos(sign))
	      return val_binary(&V, VAL_OP_TIMES, VAL_M_ONE, L);
	  break;
      }

      case VAL_OP_INT:
      case VAL_OP_NINT:
      case VAL_OP_REAL:
      case VAL_OP_DOUBLE:
	if (ve_expType(V[L]) == ve_expType(*value)) return L;
	break;

      default:
	break;
    }

    return VAL_NIL;	// not simplified
}

static ValNumber simplify_plus(ValTable &V,
			       ValEntry *value)
{
    ValNumber L, R;

    val_order(V, value);

    L = ve_left(*value);	// may be constant
    R = ve_right(*value);	// must be nonconstant now

    if (L == VAL_ZERO) return R;

    /*
     *	Note that sorting/ordering ignores constant coefficients
     *		on terms.
     */
    if (is_sum(V,L))		// both are sums
    {
	return sorted_merge(V, VAL_OP_PLUS, L, R);
    }
    else if (is_sum(V,R))	// left is non-sum, right is sum
    {
	unsigned long scoreL  = score(V,L);
	unsigned long scoreLR = score(V, ve_left(V[R]));

	if (scoreL == scoreLR)	// combine terms (may have diff. coeffs)
	{
	    ve_left(*value) = combine(V, VAL_OP_PLUS,
				      L, ve_left(V[R]));

	    if (ve_left(*value) == VAL_ZERO) return ve_right(V[R]);

	    ve_right(*value) = ve_right(V[R]);
	}
	else if (scoreL < scoreLR)	// push L down into sum
	{
	    ve_left(*value) = ve_left(V[R]);

	    ve_right(*value) = val_binary(&V, VAL_OP_PLUS,
					  L, ve_right(V[R]));

	    if (ve_type(V[ve_right(*value)]) == VAL_CONST)
	    {
		// successful cancellation, top level may now be disordered

		return val_simplify(V, value);
	    }
	}
	return VAL_NIL;		// simplified
    }
    else			// both are non-sums
    {
	unsigned long scoreL = score(V,L);
	unsigned long scoreR = score(V,R);

	if (scoreL == scoreR)	// combine terms (may have diff. coeffs)
	{
	    return combine(V, VAL_OP_PLUS, L, R);
	}
	else return VAL_NIL;	// already sorted by val_order
    }
} /* end simplify_plus */
    
static ValNumber simplify_times(ValTable &V,
				ValEntry *value)
{
    ValNumber L, R;

    val_order(V, value);

    L = ve_left(*value);
    R = ve_right(*value);

    if (L == VAL_ZERO) return VAL_ZERO;
    if (L == VAL_ONE)  return R;

    if (is_term(V,L))	// both are terms
    {
	return sorted_merge(V, VAL_OP_TIMES, L, R);
    }
    else if (is_term(V,R))	// left is non-term, right is term
    {
	// if is_sum(V,L), should distribute R over L, but punt for now

	unsigned long scoreL  = score(V,L);
	unsigned long scoreLR = score(V, ve_left(V[R]));

	if (scoreL == scoreLR)		// combine (constant?) factors
	{
	    L = combine(V, VAL_OP_TIMES, L, ve_left(V[R]));

	    if (L != VAL_NIL)		// combined constant factors
	    {
		ve_left(*value) = L;
		ve_right(*value) = ve_right(V[R]);
	    }
	}
	else if (scoreL < scoreLR)	// push L into term
	{
	    ve_left(*value) = ve_left(V[R]);

	    ve_right(*value) = val_binary(&V, VAL_OP_TIMES,
					  L, ve_right(V[R]));
	}
	return VAL_NIL;		// simplified
    }
    else if ((ve_type(V[L]) == VAL_CONST) && is_sum(V,R))
    {
	// distribution of constant over sum
	//
	ValNumber newL, newR;
	newL = val_binary(&V, VAL_OP_TIMES, L, ve_left(V[R]));
	newR = val_binary(&V, VAL_OP_TIMES, L, ve_right(V[R]));
	return val_binary(&V, VAL_OP_PLUS, newL, newR);
    }
    else			// both are non-terms -- punting on sums
    {
	return VAL_NIL;		// already sorted by val_order
    }
} /* end simplify_times */



static unsigned long score(ValTable &V, ValNumber vn)
{
    if (ve_type(V[vn]) == VAL_CONST) return MAX_SCORE;

    /*
     *  Strip off coefficient -- side effect of function on vn.
     */
    (void) extract_coeff(V, vn);

    /*
     *  High scores go to most-variant (deepest level)
     *
     *  Assumptions:
     *		-- invariant level is 0
     *		-- deepest level is highest level number
     *		-- ValNumbers never get so high as to have upper 6 bits set
     *		-- level numbers never more than 6 bits (always < 64)
     */
    const unsigned int shift = (sizeof(ValNumber)*8)-6;

    long level = ve_level(V[vn]);
    if (!((vn < (1L << shift)) &&
	  (((level << shift)>>shift) == level))
	)
	die_with_message("score: problem with levels\n");

    unsigned long sc = vn | (level << shift);

    if (sc == MAX_SCORE)	// don't want competition with constants
	die_with_message("score: too high\n");

    return sc;
}


static ValNumber combine(ValTable &V, ValOpType op,
			 ValNumber v1, ValNumber v2)
{
    if (op == VAL_OP_PLUS)
    {
	if ((ve_type(V[v1]) == VAL_CONST) && (ve_type(V[v2]) == VAL_CONST))
	{
	    return combine_constants(V, VAL_OP_PLUS, TYPE_INTEGER,
				     ve_const(V[v1]), ve_const(V[v2]));
	}
	/*
	 *  Parse into c1*v1 + c2*v2
	 */
	int c1 = extract_coeff(V, v1);
	int c2 = extract_coeff(V, v2);

	if (v1 != v2)
	    die_with_message("combine: attempt made with different terms\n");

	ValNumber coeff = combine_constants(V, VAL_OP_PLUS, 
					    TYPE_INTEGER, c1, c2);

	if (coeff == VAL_ZERO) return VAL_ZERO;
	if (coeff == VAL_ONE)  return v1;
	// else
	return val_binary(&V, VAL_OP_TIMES, coeff, v1);
    }
    // else (op == VAL_OP_TIMES)

    if ((ve_type(V[v1]) == VAL_CONST) && (ve_type(V[v2]) == VAL_CONST))
    {
	return combine_constants(V, VAL_OP_TIMES, TYPE_INTEGER,
				 ve_const(V[v1]), ve_const(V[v2]));
    }
    if (v1 != v2)
	die_with_message("combine: internal error?\n");

    return VAL_NIL;	// don't build v1 squared -- score would be different
} // end combine


static ValNumber sorted_merge(ValTable &V, ValOpType op,
			      ValNumber v1, ValNumber v2)
{
    if ((ve_type(V[v1]) != VAL_OP) ||
	(ve_type(V[v2]) != VAL_OP) ||
	(ve_opType(V[v1]) != op) ||
	(ve_opType(V[v2]) != op))
    {
	// We shouldn't get here through call from outside -- that would
	// cause infinite recursion with val_binary.  This is to stop
	// recursion from inside.

	return val_binary(&V, op, v1, v2);
    }
    else if (score(V, ve_left(V[v1])) < score(V, ve_left(V[v2])))
    {
	return val_binary(&V, op, ve_left(V[v2]), 
			  sorted_merge(V, op, v1, ve_right(V[v2])));
    }
    else // (score(v1 left) >= score(v2 left))
    {
	return val_binary(&V, op, ve_left(V[v1]), 
			  sorted_merge(V, op, ve_right(V[v1]), v2));
    }
}





#if _INTRINSICS_OK_
      case INT_FCN:  /* conversion to integer */
      case REAL_FCN: /* conversion to real */
      case DBLE_FCN: /* conversion to double */
      case CMPLX_FCN: /* conversion to complex */
      case ICHAR_FCN: /* convert character array to integer */
      case CHAR_FCN:  /* convert integer to character */
      case AINT_FCN:  /* truncation */
      case ANINT_FCN: /* nearest whole number */
      case NINT_FCN:  /* nearest integer */
      case ABS_FCN:   /* absolute value */
	if c >= 0 return c else -c

      case MOD_FCN:   /* modulus */
	if a == b return 0
	else return mod(a,b)  // euclidean mod or % ?

      case SIGN_FCN: /* transfer sign(kid2) to kid1 */
	if a == b return a
	if both >= 0 or both <= 0 return a else -a

      case DIM_FCN:  /* positive difference */
	if a == b return 0
	else if a > b return a-b else 0

      lots of silly stuff to do with real intrinsics but would they
      ever pay off?
#endif



//  range_simplify
//	Simplify binary op with constant/range arguments to constant/range --
//	constant/range means constant, bottom, or range with constant bounds.
//  
static ValNumber range_simplify(ValTable &V, ValEntry *value)
{
    ValNumber L, U, A, S;
    ValSign comp;

    switch(ve_type(*value))
    {
      case VAL_OP_MINUS:
	ve_opType(*value) = VAL_OP_PLUS;
        ve_right(*value) = val_binary(&V, VAL_OP_TIMES,
				      VAL_M_ONE, ve_right(*value));
	// don't break... fall through

      case VAL_OP_PLUS:
	L = val_min(&V, 
		    val_get_lo(&V, ve_left(*value)), 
		    val_get_lo(&V, ve_right(*value)));
	U = val_max(&V, 
		    val_get_hi(&V, ve_left(*value)), 
		    val_get_hi(&V, ve_right(*value)));
	S = val_gcd(&V,
		    val_get_step(&V, ve_left(*value)),
		    val_get_step(&V, ve_right(*value)));
	A = val_emod(&V,
		     val_binary(&V, VAL_OP_PLUS,
				val_get_align(&V, ve_left(*value)),
				val_get_align(&V, ve_right(*value))),
		     S);
	return val_lookup_range(V, L, U, A, S);

      case VAL_OP_TIMES:
	//
	//  Only handle simple case, where one is constant, for now
	//
	ValNumber cval, rval;
	if (ve_type(V[ve_left(*value)]) == VAL_CONST)
	{
	    cval = ve_left(*value);
	    rval = ve_right(*value);
	}
	else if (ve_type(V[ve_right(*value)]) == VAL_CONST)
	{
	    cval = ve_right(*value);
	    rval = ve_left(*value);
	}
	else return VAL_BOTTOM;

	L = val_binary(&V, VAL_OP_TIMES, cval, val_get_lo(&V, rval));
	U = val_binary(&V, VAL_OP_TIMES, cval, val_get_hi(&V, rval));
	A = val_binary(&V, VAL_OP_TIMES, cval, val_get_align(&V, rval));
	S = val_binary(&V, VAL_OP_TIMES, cval, val_get_step(&V, rval));

	return val_lookup_range(V, L, U, A, S);

      case VAL_OP_MIN:
	comp = val_compare(&V, ve_left(*value), ve_right(*value));
	if (VAL_is_le(comp)) return ve_left(*value);
	if (VAL_is_ge(comp)) return ve_right(*value);
	L = val_min(&V, 
		    val_get_lo(&V, ve_left(*value)), 
		    val_get_lo(&V, ve_right(*value)));
	U = val_min(&V, 
		    val_get_hi(&V, ve_left(*value)), 
		    val_get_hi(&V, ve_right(*value)));
	val_merge_steps(V, S, A, 
			val_get_step(&V, ve_left(*value)),
			val_get_step(&V, ve_right(*value)),
			val_get_align(&V, ve_left(*value)),
			val_get_align(&V, ve_right(*value)));
	return val_lookup_range(V, L, U, A, S);

      case VAL_OP_MAX:
	comp = val_compare(&V, ve_left(*value), ve_right(*value));
	if (VAL_is_ge(comp)) return ve_left(*value);
	if (VAL_is_le(comp)) return ve_right(*value);
	L = val_max(&V, 
		    val_get_lo(&V, ve_left(*value)), 
		    val_get_lo(&V, ve_right(*value)));
	U = val_max(&V, 
		    val_get_hi(&V, ve_left(*value)), 
		    val_get_hi(&V, ve_right(*value)));
	val_merge_steps(V, S, A, 
			val_get_step(&V, ve_left(*value)),
			val_get_step(&V, ve_right(*value)),
			val_get_align(&V, ve_left(*value)),
			val_get_align(&V, ve_right(*value)));
	return val_lookup_range(V, L, U, A, S);

      case VAL_OP_DIVIDE:
	if (ve_type(V[ve_right(*value)]) != VAL_CONST)
	{
	    return VAL_BOTTOM;
	}
	else
	{
	    A = val_get_align(&V, ve_left(*value));
	    S = val_get_step(&V, ve_left(*value));
	    ValNumber d = val_gcd(&V, A, S);
	    if (val_mod(&V, d, ve_right(*value)) == VAL_ZERO)
	    {
		S = val_binary(&V, VAL_OP_DIVIDE, S, ve_right(*value));
		A = val_emod(&V, A, S);
	    }
	    int c = val_get_const(&V, ve_right(*value));
	    if (c > 0)
	    {
		L = val_div_ceil(&V, L, ve_right(*value));
		U = val_div_floor(&V, U, ve_right(*value));
	    }
	    else if (c < 0)
	    {
		L = val_div_ceil(&V, U, ve_right(*value));
		U = val_div_floor(&V, L, ve_right(*value));
	    }
	    else
	    {
		die_with_message("simplify_range: divide by 0\n");
	    }
	    return val_lookup_range(V, L, U, A, S);
	}
	break;

      case VAL_OP_MOD:
	// Should be able to handle these two in many interesting cases,
	// but fall through to default for now.
      default:
	return VAL_BOTTOM;
    }
}
