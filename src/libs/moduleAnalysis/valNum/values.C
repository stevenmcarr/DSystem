/* $Id: values.C,v 2.11 1997/03/11 14:36:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/frontEnd/ast/ast.h>

const char *valType[] = { "VAL_TOP_TYPE", "VAL_CONST", "VAL_TEXT", "VAL_RANGE",
			  "VAL_IVAR", "VAL_VARIANT", "VAL_ENTRY", "VAL_RETURN",
			  "VAL_IP_BASE",
			  "VAL_OP", "VAL_INTRINSIC", "VAL_PURE_FN", "VAL_PHI",
			  "VAL_GAMMA", "VAL_LIST", "VAL_TEST", "VAL_ETA", 
			  "VAL_OK_MU", "VAL_VAR_MU", "VAL_MU_PH", "VAL_DUMMY", 
			  "VAL_ARRAY", "VAL_BOT_TYPE",
			  "VAL_OP_PLUS", "VAL_OP_TIMES", "VAL_OP_MINUS",
			  "VAL_OP_DIVIDE", "VAL_OP_NOT", "VAL_OP_AND",
			  "VAL_OP_OR", "VAL_OP_NE", "VAL_OP_GE", "VAL_OP_GT",
			  "VAL_OP_LE", "VAL_OP_LT", "VAL_OP_EQ", "VAL_OP_NE",
			  "VAL_OP_EXP", "VAL_OP_ABS", "VAL_OP_MOD",
			  "VAL_OP_TYPE", "VAL_OP_MAX", "VAL_OP_MIN",
			  "VAL_OP_DIM", "VAL_OP_SIGN", "VAL_OP_LEN" };

const char **valOpType = valType;	// This helps us keep VAL_* and VAL_OP_*
					// disjoint.

/****************************************************************************
 **		    FUNCTIONS GLOBAL TO THE WORLD                          **
 ****************************************************************************/


/*---------------------------------------------------------------------------
 * -- val_unary
 *  
 *           Build symbolic value entry for unary TREE_OPs
 *
 *--------------------------------------------------------------------------*/

ValNumber val_unary(Values Vp,
		    ValOpType op,
		    ValNumber valNum)
{
    if ((valNum == VAL_TOP) || (valNum == VAL_BOTTOM) )
	return(valNum);

    if (op == VAL_OP_MINUS)
	return val_binary(Vp, VAL_OP_TIMES, VAL_M_ONE, valNum);

    return val_lookup_op(*Vp, val_op_exptype(*Vp, op, valNum, VAL_NIL),
			 op, /* arity = */ 1, &valNum);
}





/*---------------------------------------------------------------------------
 * -- val_binary
 *  
 *           Build symbolic value entry for binary TREE_OPs
 *
 *--------------------------------------------------------------------------*/

ValNumber val_binary(Values Vp,
		     ValOpType op, 
		     ValNumber valNum1, 
		     ValNumber valNum2)
{
    ValNumber	kids[2];
    
    if ((valNum1 == VAL_TOP) || (valNum2 == VAL_TOP) )
	return(VAL_TOP);
    
    if ((valNum1 == VAL_BOTTOM) || (valNum2 == VAL_BOTTOM) )
	return(VAL_BOTTOM);

    kids[0] = valNum1;
    kids[1] = valNum2;
    return val_lookup_op(*Vp, val_op_exptype(*Vp, op, valNum1, valNum2),
			 op, /* arity = */ 2, kids);
}

 

ValNumber val_const(Values Vp,
		    int constVal)
{
    return val_lookup_const(*Vp, TYPE_INTEGER, constVal);
}



/*---------------------------------------------------------------------------
 * -- val_is_unknown
 *
 *          This function returns TRUE if the value node is BOTTOM
 *          otherwise returns false
 *--------------------------------------------------------------------------*/
Boolean val_is_unknown(Values Vp,
		       ValNumber valNum)
{
    return BOOL(valNum == VAL_BOTTOM);
}
 




/*---------------------------------------------------------------------------
 * -- val_not_exec
 *
 *          This function returns TRUE if the value node is TOP (unexecutable)
 *          otherwise returns false
 *--------------------------------------------------------------------------*/
Boolean val_not_exec(Values Vp,
		     ValNumber valNum)
{
    return BOOL(valNum == VAL_TOP);
}
 





/*---------------------------------------------------------------------------
 * -- val_is_const
 *
 *          This function returns TRUE if the value node is a constant
 *          otherwise returns false
 *--------------------------------------------------------------------------*/
Boolean val_is_const(Values Vp,
		     ValNumber valNum)
{
    if (!val_is_value(valNum))
	return(false);

    return BOOL(ve_type((*Vp)[valNum]) == VAL_CONST);
}
 





/*---------------------------------------------------------------------------
 * -- val_get_exp_type
 *
 *        Return the expression type 
 *--------------------------------------------------------------------------*/
ExpType val_get_exp_type(Values Vp,
			 ValNumber valNum)
{
    if (!val_is_value(valNum))
	return(TYPE_UNKNOWN);

    return ve_expType((*Vp)[valNum]);
}







/*---------------------------------------------------------------------------
 * -- val_get_level
 *
 *        Return the loop-variant level of the value
 *--------------------------------------------------------------------------*/
int val_get_level(Values Vp,
		  ValNumber valNum)
{
    if (!val_is_value(valNum))
	return(0);

    return (int) ve_level((*Vp)[valNum]);
}







/*---------------------------------------------------------------------------
 * -- val_get_const
 *
 *        Return the integer constant if it is an integral
 *        For all others, VAL_NIL is returned
 *--------------------------------------------------------------------------*/
int val_get_const(Values Vp, ValNumber valNum)
{
    if ((!val_is_value(valNum)) ||
	(ve_type((*Vp)[valNum]) != VAL_CONST))
    {
	return(VAL_NIL);
    }
    else
    {
	return ve_const((*Vp)[valNum]);
    }
}
 






/*---------------------------------------------------------------------------
 * -- val_get_const_text
 *
 *        For non-integral constants the pointer is returned
 *        For all others, VAL_NIL is returned
 *--------------------------------------------------------------------------*/
char *val_get_const_text(Values Vp,
		       ValNumber valNum)
{
    if ((!val_is_value(valNum)) ||
	(ve_type((*Vp)[valNum]) != VAL_TEXT))
    {
	return(NULL);
    }

    /*
     *  Need to change this to table lookup
     */
    return ve_string((*Vp)[valNum]);
}
 





/****************************************************************************
 **	         FUNCTIONS PRIVATE TO VALUE GRAPH BUILDERS                 **
 ****************************************************************************/


/*---------------------------------------------------------------------------
 * -- val_swap() 
 *
 *         This function swaps two value numbers
 *--------------------------------------------------------------------------*/
void val_swap(ValNumber *val1,
	      ValNumber *val2)
{
    ValNumber temp;
    
    temp = *val1;
    *val1 = *val2;
    *val2 = temp;

} /* end of val_swap() */





/*---------------------------------------------------------------------------
 * -- val_op_exptype() 
 *       
 *       This function returns the exptype of certain operation,
 *       mostly those found in the tree, but a few implemented by intrinsics.
 *       A bit fast and loose; we're not the type checker.
 *--------------------------------------------------------------------------*/
ExpType val_op_exptype(ValTable &V, ValOpType op,
		       ValNumber left, ValNumber right)
{
    ExpType etL, etR;

    // if one is not executed, the expr is not 
    //
    if ((left == VAL_TOP) || (right == VAL_TOP)) return TYPE_UNKNOWN;

    if (right == VAL_NIL)	// unary
    {
	if ((op == VAL_OP_INT) || (op == VAL_OP_NINT) || (op == VAL_OP_LEN))
	    return TYPE_INTEGER;

	if (op == VAL_OP_REAL)   return TYPE_REAL;
	if (op == VAL_OP_DOUBLE) return TYPE_DOUBLE_PRECISION;
	
	return ve_expType(V[left]);
    }
    etR = ve_expType(V[right]);
    etL = ve_expType(V[left]);

    if ((etR == TYPE_UNKNOWN) || (etL == TYPE_UNKNOWN)) return TYPE_UNKNOWN;

    switch(op)
    {
      case VAL_OP_PLUS:
      case VAL_OP_MINUS:
      case VAL_OP_TIMES:
      case VAL_OP_DIVIDE:
      case VAL_OP_EXP:
      case VAL_OP_MOD:
      case VAL_OP_MAX:
      case VAL_OP_MIN:
      case VAL_OP_DIM:
      case VAL_OP_SIGN:
	/*
	 *  this works for numeric types, giving most general of the two
	 */
	return max(etR, etL);

      case VAL_OP_GE:
      case VAL_OP_GT:
      case VAL_OP_LE:
      case VAL_OP_LT:
      case VAL_OP_NE:
      case VAL_OP_EQ:
      case VAL_OP_AND:
      case VAL_OP_OR:
	return TYPE_LOGICAL;

      default:
	return TYPE_UNKNOWN;
    }
}  /* end of val_op_exptype() */



ValNumber val_convert(ValTable *Vp, ValNumber v, ExpType et)
{
    ValOpType ot;

    if (et == val_get_exp_type(Vp, v)) return v;

    switch (et)
    {
      case TYPE_INTEGER:		ot = VAL_OP_INT;	break;
      case TYPE_REAL:			ot = VAL_OP_REAL;	break;
      case TYPE_DOUBLE_PRECISION:	ot = VAL_OP_DOUBLE;	break;
      default:
	fprintf(stderr, "val_convert: %s",
			 "Conversion to non-numeric or non-scalar type");
	return VAL_BOTTOM;
	break;
    }
    return val_lookup_op(*Vp, et, ot, /* arity = */ 1, &v);
}




/****************************************************************************
 **			   PRIVATE FUNCTION                                **
 ****************************************************************************/


#ifdef boo
static void val_mark(Set marked,
		     ValNumber valNum)
{
    if ((valNum == VAL_NIL) || (valNum == VAL_TOP) || 
	(valNum == VAL_BOTTOM) )
    {
	return;
    }
    add_number(marked, valNum);
}

#endif



ValType val_get_val_type(Values Vp,
			 ValNumber vn)
{
    if ((vn > VAL_NIL) && (vn < val_table_max(Vp)))
	return ve_type((*Vp)[vn]);
    else
	return VAL_BOT_TYPE;
}



/*---------------------------------------------------------------------------
 * -- val_get_subs
 *
 *        Return the subscript value number if it is an array
 *--------------------------------------------------------------------------*/
ValNumber val_get_subs(Values Vp,
		       ValNumber valNum)
{
    if ((!val_is_value(valNum)) ||
	(ve_type((*Vp)[valNum]) != VAL_ARRAY))
    {
	return(VAL_NIL);
    }
    else
    {
	return ve_subs((*Vp)[valNum]);
    }
}  /* end of val_get_subs() */
