/* $Id: val_lookup.C,v 2.12 1997/03/11 14:36:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************************
 * -- val_lookup.C
 * 
 *         This file contains functions for looking up a value node
 * 
 *         By: Tim Mullin                   July 1991
 *
 *	   Modified March 1993 by Paul Havlak
 *		Switch to C++.  val_lookup_XX allocates a ValEntry of
 *		appropriate type; val_lookup() either adds it to the 
 *		ValTable or frees it.
 ****************************************************************************/

#include <stdlib.h>

#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/frontEnd/ast/builtins.h>

static ValNumber examine_kids(ValNumber kids[], int arity);
static Boolean check_integral(ExpType expType, char *name, int &intVal);

/*---------------------------------------------------------------------------
 * -- val_lookup()
 *
 *	simplify the value
 *	look it up in the ValTable
 *	if found
 *		return that value number
 *	else
 *		if insert
 *			add the value and return value number
 *		else
 *			return VAL_NIL
 *		fi
 *	fi
 *--------------------------------------------------------------------------*/
ValNumber val_lookup(ValTable &V,
                     ValEntry *value,
                     Boolean insert)
{
    ValNumber	valNum = VAL_NIL;

    if (VAL_bogus(value))
    {
	valNum = VAL_BOTTOM;
    }
    else
	valNum = V[value];

    if (valNum == VAL_NIL)	// was not already in table
    {
	if (insert)
	{
	    V.add_entry(value);
	    valNum = V[value];

	    ve_cRange(*value) = val_get_const_range(&V, valNum);

	    if ((valNum % 10000) == 0) 
	    { 
		fprintf(stderr, "%d entries in ValTable %p\n", valNum, &V);
	    }
	    return valNum;
	}
    }
    //  If still here, passed object is no longer needed
    //  (value already entered, or not found and not inserted\)
    //
    val_delete(value);

    return valNum;

} /* end of val_lookup() */




/*---------------------------------------------------------------------------
 * -- val_lookup_const() 
 *
 *          This function looks up a constant in the value table
 *--------------------------------------------------------------------------*/
ValNumber val_lookup_const(ValTable &V,
			   ExpType expType,
			   int constval,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_CONST);

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = 0;
    ve_const(*myValue)   = constval;

    return val_lookup(V, myValue, insert);
}


/*---------------------------------------------------------------------------
 * -- val_lookup_text() 
 *
 *          This function looks up a constant based on its text representation.
 *--------------------------------------------------------------------------*/
ValNumber val_lookup_text(ValTable &V,
			  ExpType expType,
			  char *name,
			  Boolean insert)
{
    int intVal;

    if (((expType == TYPE_REAL) ||
	 (expType == TYPE_DOUBLE_PRECISION) ||
	 (expType == TYPE_LOGICAL)) &&
	check_integral(expType, name, intVal))
    {
	return val_lookup_const(V, expType, intVal);
    }

    ValEntry *myValue = val_new(VAL_TEXT, strlen(name));

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = 0;
    /*
     *  Need to do string table lookup here to get StrId
     */
    strcpy(ve_string(*myValue), name);
    
    return val_lookup(V, myValue, insert);
}




/*---------------------------------------------------------------------------
 * -- val_lookup_phi
 *  
 *      Build symbolic value for a phi (value merge)
 *  
 *	Bogus for self-dependent phis.  Use val_lookup_mu for those.
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_phi(ValTable &V,
			 ExpType expType,
			 ValField stmt,
			 unsigned int arity,
			 ValNumber kids[],
			 Boolean insert)
{
    ValNumber kidval = examine_kids(kids, arity);

    if (kidval != VAL_NIL) return kidval;

    ValEntry *myValue = val_new(VAL_PHI, arity);

    ve_expType(*myValue) = expType;
    ve_stmt(*myValue)    = stmt;
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = 0;

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue)  = max(ve_level(*myValue),
				  ve_level(V[kids[i]]));
	ve_kid(*myValue,i)  = kids[i];
    }

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_eta
 *  
 *           Build symbolic value for an eta (loop exit value)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_eta(ValTable &V,
			 ValNumber test,
			 ValNumber final,
			 Boolean insert)
{
    ValEntry *myValue = val_new(VAL_ETA);

    ve_expType(*myValue) = ve_expType(V[final]);
    /*
     *  If the test doesn't vary with the loop, shouldn't be here
     */
    ve_level(*myValue)   = ve_level(V[test]) -1;
    ve_test(*myValue)    = test;
    ve_final(*myValue)   = final;
    /*
     *  Should sometimes be able to simplify to last value in loop
     */
    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_test
 *  
 *           Build symbolic value for an strange branch
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_test(ValTable &V,
			  ValNumber itest,
			  int type,
			  ValField occur,
			  Boolean insert)
{
    ValEntry *myValue = val_new(VAL_TEST);

    ve_expType(*myValue)   = ve_expType(V[itest]);
    ve_level(*myValue)     = ve_level(V[itest]);
    ve_itest(*myValue)     = itest;
    ve_testType(*myValue)  = type;
    ve_occur(*myValue)     = occur;
    /*
     *  Should be able to simplify sometimes, for the easier 
     *  varieties of branch types.
     */
    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_gamma
 *  
 *           Build symbolic value for a gamma (controlled value merge)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_gamma(ValTable &V,
			   ExpType expType,
			   ValNumber test,
			   unsigned int arity,
			   ValNumber kids[],
			   Boolean insert)
{
    /*
     *  If lower bound of test > 0, upper bound <= arity,
     *  or stride non-zero, then set corresponding kids to VAL_TOP.
     *  If that leaves only one non-TOP kid, examine_kids will 
     *  return it.
     *  for (i = 0; i < arity; i++)
     *      if (i not in ve_cRange(V[test]))
     *          kid[i] = VAL_TOP;
     */
    ValNumber kidval = examine_kids(kids, arity);

    if (val_is_value(kidval))
	return kidval;

    if ((expType == TYPE_LOGICAL) &&
	(ve_expType(V[test]) == TYPE_LOGICAL) &&
	(arity == 2))
    {
	if ((kids[0] == VAL_TRUE) && (kids[1] == VAL_FALSE)) return test;

	if ((kids[0] == VAL_FALSE) && (kids[1] == VAL_TRUE)) 
	    return val_unary(&V, VAL_OP_NOT, test);
    }

    ValEntry *myValue = val_new(VAL_GAMMA, arity);

    ve_expType(*myValue) = expType;
    ve_test(*myValue)    = test;
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = ve_level(V[test]);

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue) = max(ve_level(*myValue),
				 ve_level(V[kids[i]]));
	ve_kid(*myValue,i) = kids[i];
    }

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_pure
 *  
 *           Build symbolic value for a pure user fn (no Mods, no global Refs)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_pure(ValTable &V,
			  ExpType expType,
			  char *name,
			  unsigned int arity,
			  ValNumber kids[],
			  Boolean insert)
{
    ValEntry *myValue = val_new(VAL_PURE_FN, arity);

    ve_expType(*myValue) = expType;
    ve_entry(*myValue)   = val_lookup_text(V, TYPE_CHARACTER, name);
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = 0;

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue) = max(ve_level(*myValue),
				 ve_level(V[kids[i]]));
	ve_kid(*myValue,i) = kids[i];
    }

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_intrinsic
 *  
 *           Build symbolic value for return value of intrinsic
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_intrinsic(ValTable &V,
			       ExpType expType,
			       char *name,
			       unsigned int arity,
			       ValNumber kids[],
			       Boolean insert)
{
    ValEntry *myValue = val_new(VAL_INTRINSIC, arity);

    ve_expType(*myValue) = expType;
    /*
     *  Need to lookup StrId for name...
     *  If name is NULL, this is just type conversion and entry -1 for StrId
     */
    ve_entry(*myValue)   = val_lookup_text(V, TYPE_CHARACTER, name);
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = 0;

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue) = max(ve_level(*myValue),
				 ve_level(V[kids[i]]));
	ve_kid(*myValue,i) = kids[i];
    }

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_op
 *  
 *           Build symbolic value for a gamma (controlled value merge)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_op(ValTable &V,
			ExpType expType,
			ValOpType opType,
			unsigned int arity,
			ValNumber kids[],
			Boolean insert)
{
    ValEntry *myValue = val_new(VAL_OP, arity);

    ve_expType(*myValue) = expType;
    ve_opType(*myValue)  = opType;
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = 0;

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue) = max(ve_level(*myValue),
				 ve_level(V[kids[i]]));
	ve_kid(*myValue,i) = kids[i];
    }

    ValNumber valNum = val_simplify(V, myValue);

    if (valNum == VAL_NIL)
    {
	return val_lookup(V, myValue, insert);
    }
    else
    {
	val_delete(myValue);
	return valNum;
    }
}





/*---------------------------------------------------------------------------
 * -- val_lookup_ok_mu
 *  
 *           Build symbolic value for a mu (value merge at loop entry)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_ok_mu(ValTable &V,
			   ValNumber init,
			   ValNumber iter,
			   int level,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_OK_MU);

    ve_expType(*myValue) = ve_expType(V[init]);
    ve_level(*myValue)   = level;
    ve_init(*myValue)    = init;
    ve_iter(*myValue)    = iter;	// nice iterative value
    /*
     *  Should check for nice symbolic increment and do auxiliary 
     *  induction variable substitution.
     */
    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_var_mu
 *  
 *           Build symbolic value for a mu (value merge at loop entry)
 *		with messy iterative value
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_var_mu(ValTable &V,
			    ValField occur,
			    ValNumber init,
			    int level,
			    Boolean insert)
{
    ValEntry *myValue = val_new(VAL_VAR_MU);

    ve_expType(*myValue) = ve_expType(V[init]);
    ve_level(*myValue)   = level;
    ve_occur(*myValue)   = occur;	// should be unique
    ve_init(*myValue)    = init;
    ve_varIter(*myValue) = VAL_NIL;	// temporary

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_mu_ph
 *  
 *           Build placeholder for Mu used in building iterative value
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_mu_ph(ValTable &V,
			   int level,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_MU_PH);

    ve_expType(*myValue) = TYPE_INTEGER;	// ok, assume something here
    ve_level(*myValue)   = level;

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_variant
 *  
 *           Build symbolic value for a phi node
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_variant(ValTable &V,
			     ExpType expType,
			     ValField occur,
			     int level,
			     Boolean insert)
{
    ValEntry *myValue = val_new(VAL_VARIANT);

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = level;
    ve_occur(*myValue)   = occur;

    return val_lookup(V, myValue, insert);
}




/*---------------------------------------------------------------------------
 * -- val_lookup_entry
 *  
 *           Build symbolic value for a (formal, global, or static) 
 *	     variable on entry
 *
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_entry(ValTable &V,
			   ExpType expType,
			   char *entry,
			   char *name,
			   int offset,
			   int length,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_ENTRY);

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = 0;		// can never call into a loop

    ve_entry(*myValue)  = val_lookup_text(V, TYPE_CHARACTER, entry);
    ve_name(*myValue)	= val_lookup_text(V, TYPE_CHARACTER, name);
    ve_offset(*myValue) = offset;
    ve_length(*myValue) = length;

    return val_lookup(V,  myValue, insert);
}




/*---------------------------------------------------------------------------
 * -- val_lookup_ip_base
 *  
 *           Build symbolic value for a (formal, global, or static) 
 *	     variable on entry (base for several related values)
 *
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_ip_base(ValTable &V,
			     ExpType expType,
			     char *entry,
			     char *name,
			     int offset,
			     int length,
			     Boolean insert)
{
    ValEntry *myValue = val_new(VAL_IP_BASE);

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = 0;		// can never call into a loop

    ve_entry(*myValue)  = val_lookup_text(V, TYPE_CHARACTER, entry);
    ve_name(*myValue)	= val_lookup_text(V, TYPE_CHARACTER, name);
    ve_offset(*myValue) = offset;
    ve_length(*myValue) = length;

    return val_lookup(V,  myValue, insert);
}




/*---------------------------------------------------------------------------
 * -- val_lookup_return
 *  
 *           Build symbolic value for a (actual, global, or static) 
 *	     variable on return
 *
 *	     (input is the value in the global coming into the call site)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_return(ValTable &V,
			    ExpType expType,
			    int level,
			    ValField call,
			    char *name,
			    int offset,
			    int length,
			    ValNumber input,
			    Boolean insert)
{
    ValEntry *myValue = val_new(VAL_RETURN);

    ve_expType(*myValue) = expType;
    ve_level(*myValue)   = level;

    ve_call(*myValue)   = call;
    ve_name(*myValue)	= val_lookup_text(V, TYPE_CHARACTER, name);
    ve_offset(*myValue) = offset;
    ve_length(*myValue) = length;
    ve_input(*myValue)  = input;

    return val_lookup(V,  myValue, insert);
}







/*---------------------------------------------------------------------------
 * -- val_lookup_range
 *  
 *           Build symbolic value for a range (e.g., loop bounds)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_range(ValTable &V,
                           ValNumber lo,
                           ValNumber hi,
                           ValNumber align,
                           ValNumber step,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_RANGE);

    ve_expType(*myValue) = TYPE_INTEGER;
    ve_level(*myValue)   = max(ve_level(V[lo]),
			       ve_level(V[hi]));
    ve_level(*myValue)   = max(ve_level(V[align]),
			       ve_level(*myValue));
    ve_level(*myValue)   = max(ve_level(V[step]),
			       ve_level(*myValue));

    ve_lo(*myValue)     = lo;
    ve_hi(*myValue)     = hi;
    ve_align(*myValue)  = align;
    ve_step(*myValue)   = step;
    ve_simple(*myValue) = (ValField) false;

    ValNumber valNum = val_standardize(V, myValue);	// make forward range

    if (valNum == VAL_NIL)
    {
	return val_lookup(V, myValue, insert);
    }
    else
    {
	val_delete(myValue);
	return valNum;
    }
}




/*---------------------------------------------------------------------------
 * -- val_lookup_ivar() 
 *
 *          This function looks up a loop index variable
 *--------------------------------------------------------------------------*/
ValNumber val_lookup_ivar(ValTable &V,
			  int level,
			  ValNumber range,
			  Boolean flipped,
			  Boolean insert)
{
    ValEntry *myValue = val_new(VAL_IVAR);

    ve_expType(*myValue) = ve_expType(V[range]);
    ve_level(*myValue)   = level;
    ve_bounds(*myValue)  = range;
    ve_flipped(*myValue) = flipped;

    return val_lookup(V, myValue, insert);
}




/*---------------------------------------------------------------------------
 * -- val_lookup_dummy
 *  
 *           Reference to hidden or unmodified global variable.
 *		"state" is reaching "definition" of DUMMY_GLOBAL.
 *		-- view this as a selector from global state
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_dummy(ValTable &V,
			   ExpType expType,
			   ValNumber state,
			   char *name,
			   int offset,
			   int length,
			   Boolean insert)
{
    ValEntry *myValue = val_new(VAL_DUMMY);

    ve_expType(*myValue) = expType;
    ve_state(*myValue)   = state;
    ve_level(*myValue)   = ve_level(V[state]);

    ve_name(*myValue)	= val_lookup_text(V, TYPE_CHARACTER, name);
    ve_offset(*myValue) = offset;
    ve_length(*myValue) = length;

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_array
 *  
 *           Subscripted array references
 *		whole array value (subscripted def) has non-nil RHS
 *		partial array value (subscripted use) has VAL_NIL RHS
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_array(ValTable &V,
			   ExpType expType,
			   ValNumber state,
			   ValNumber rhs,
			   ValNumber subs,
			   Boolean insert)
{
    //  Simplifications... if subscripts are same as that of reaching
    //  definition...
    //
    if ((ve_type(V[state]) == VAL_ARRAY) &&
	(ve_subs(V[state]) == subs))
    {
	if (rhs != VAL_NIL)	// has RHS <=> is def
	{
	    //  This subscripted definition exactly overlays previous one,
	    //  so the unaffected state is the prior reaching def.
	    //
	    state = ve_state(V[state]);
	}
	else			// no RHS <=> is use
	{
	    //  This subscripted use references exactly the location(s)
	    //  defined in the immediately preceding def, so we can
	    //  substitute the RHS
	    //
	    return ve_rhs(V[state]);
	}
    }

    ValEntry *myValue = val_new(VAL_ARRAY);

    ve_expType(*myValue) = expType;
    ve_state(*myValue)   = state;
    ve_rhs(*myValue)     = rhs;
    ve_subs(*myValue)    = subs;
    ve_level(*myValue)   = ve_level(V[state]);

    ve_level(*myValue)     = max(ve_level(*myValue), ve_level(V[subs]));

    if (rhs != VAL_NIL)
	ve_level(*myValue) = max(ve_level(*myValue), ve_level(V[rhs]));

    return val_lookup(V, myValue, insert);
}





/*---------------------------------------------------------------------------
 * -- val_lookup_list
 *  
 *           Build symbolic value for a gamma (controlled value merge)
 *--------------------------------------------------------------------------*/

ValNumber val_lookup_list(ValTable &V,
			  unsigned int arity,
			  ValNumber kids[],
			  Boolean insert)
{
    ValEntry *myValue = val_new(VAL_LIST, arity);

    ve_expType(*myValue) = TYPE_UNKNOWN;
    ve_test(*myValue)    = VAL_NIL;	// unused field, for now
    ve_arity(*myValue)   = arity;
    ve_level(*myValue)   = 0;

    for (unsigned int i = 0; i < arity; i++)
    {
	ve_level(*myValue) = max(ve_level(*myValue),
				 ve_level(V[kids[i]]));
	ve_kid(*myValue,i) = kids[i];
    }
    return val_lookup(V, myValue, insert);
}





static ValNumber examine_kids(ValNumber kids[], int arity)
{
    /*
     *  -- if any kid is VAL_BOTTOM, return VAL_BOTTOM
     *
     *  -- if all kids are same or VAL_TOP, return that ValNumber
     *
     *  -- otherwise, return VAL_NIL
     */
    int i;
    ValNumber save = VAL_NIL;
    Boolean different = false;

    for (i = 0; i < arity; i++)
    {
	if (kids[i] == VAL_BOTTOM)
	{
	    return VAL_BOTTOM;
	}
	if (!different && (save == VAL_NIL))
	{
	    save = kids[i];
	}
	else if ((kids[i] != VAL_TOP) && (kids[i] != save))
	{
	    save = VAL_NIL;
	    different = true;
	    break;
	}
    }
    return save;
}

static Boolean check_integral(ExpType expType, char *name, int &intVal)
{
    switch(expType)
    {
      case TYPE_INTEGER:
	intVal = atoi(name);
	return true;
	break;

      case TYPE_LOGICAL:
	if (!strcmp(name, ".true."))
	    intVal = 1;
	else
	    intVal = 0;

	return true;
	break;

      case TYPE_REAL:
      case TYPE_DOUBLE_PRECISION:
	/*
	 *  Only problem is needing to handle "d" format in Fortran...
	 *  This approach is dangerous when precision affects whether
	 *  or not things appear to have integral values.
	 */
	if (strchr(name, 'd'))
	{
	    if (strlen(name) < 20)
	    {
		char temp[20];

		(void) strcpy(temp, name);
		char *id = strchr(temp, 'd');
		*id = 'f';
		double dVal;
		(void) sscanf(temp, "%lf", &dVal);
		intVal = (int) dVal;
		if (intVal == dVal) return true;
	    }
	}
	else
	{
	    double dVal;
	    sscanf(name, "%lf", &dVal);
	    intVal = (int) dVal;
	    if (intVal == dVal) return true;
	}
	break;

      default:
	return false;
	break;
    }
    return false;
}


//  Lookup routines for finding interprocedural values
//
ValNumber val_lookup_ip_entry(ValTable &V,
			      char *entry,
			      char *name,
			      int offset)
{
    return val_lookup_entry(V, /* expType = */ TYPE_UNKNOWN,
			    entry, name, offset, /* length = */ 0,
			    /* insert = */ false);
}

ValNumber val_lookup_ip_return(ValTable &V,
			       ValField call,
			       char *name,
			       int offset)
{
    return val_lookup_return(V, /* expType = */ TYPE_UNKNOWN,
			    /* level = */ 0,
			    call, name, offset, /* length = */ 0,
			    /* input = */ VAL_NIL,
			    /* insert = */ false);
}
