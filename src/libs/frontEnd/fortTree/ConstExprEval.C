/* $Id: ConstExprEval.C,v 1.1 1997/06/24 17:53:08 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>

#include <libs/frontEnd/include/gi.h>

#include <libs/frontEnd/fortTree/FortTree.i>
#include <libs/frontEnd/fortTree/fortsym.i>
#include <libs/frontEnd/fortTree/TypeChecker.h>

#define EV_Error -1 /* anything nonzero */

static int ErrorInEvaluation = 0;


STATIC(int, evalConstIntExpr, (SymDescriptor d, AST_INDEX expr));
STATIC(int, fastEvalConstIntExpr, (SymDescriptor d, AST_INDEX expr));


/* 
 *    evaluates a constant expression returning its value in "value"
 *    a non-zero error code is if an error is encountered during evaluation
 *    if an error is encountered, "value" is undefined
 */
int evalConstantIntExpr(SymDescriptor d, AST_INDEX expr, int* value)
{
    ErrorInEvaluation = 0;
    *value = evalConstIntExpr(d,expr); /* call recursive evaluator */
    return ErrorInEvaluation;
}


/* 
 *    evaluates a constant expression returning its value in "value"
 *    a non-zero error code is if an error is encountered during evaluation
 *    if an error is encountered, "value" is undefined
 *    
 *    This function is added by nenad (8/3/93) because cfgval_evaluate
 *    does not need all the error checking and reporting features of the
 *    original evalConstantIntExpr. 
 */
int fastEvalConstantIntExpr(SymDescriptor d, AST_INDEX expr, int* value)
{
  ErrorInEvaluation = 0;
  *value = fastEvalConstIntExpr(d,expr); /* call recursive evaluator */
  return ErrorInEvaluation;
}

 
/* 
 *    recursive evaluator for constant expressions
 *    sets "static int ErrorInEvaluation" to non-zero value if an
 *    error is encountered during evaluation
 *
 *    ASSUMPTIONS: each parameter defined in the symbol table has the
 *      symbol table field SYMTAB_EXPR defined to be the astnode 
 *      that is the head of the expression tree for the parameter's value 
 */
static int evalConstIntExpr(SymDescriptor d, AST_INDEX expr)
{
    SymTable t = d->Table;
    int value;

    switch (gen_get_node_type(expr)) {
    case GEN_CONSTANT:
		ConstantSetType(expr);
        if (gen_get_real_type(expr) != TYPE_INTEGER) {
            ErrorInEvaluation = EV_Error;
            ft_SetSemanticErrorForStatement(
                (FortTree)((TableDescriptor)(d->parent_td))->FortTreePtr, 
                 expr,  ft_NON_INT_IN_CONST_INT_EXPR);
            value = 0; /* error case evaluates to zero */
        } 
        else value = atoi(gen_get_text(expr));
        return value;;

    case GEN_IDENTIFIER:
    {
        int index = SymQueryIndex(t, gen_get_text(expr));
        if (!IS_MANIFEST_CONSTANT(t, index)) {
            ErrorInEvaluation = EV_Error;
            ft_SetSemanticErrorForStatement(
                (FortTree)((TableDescriptor)(d->parent_td))->FortTreePtr, 
                 expr,  ft_NON_CONST_IN_CONST_INT_EXPR);
            value = 0; /* error case evaluates to zero */
        } else {
            if (SymGetFieldByIndex(t, index, SYMTAB_PARAM_STATUS) == 
                    PARAM_VALUE_DEFINED)
                value = SymGetFieldByIndex(t, index, SYMTAB_PARAM_VALUE);
            else {
                if (SymGetFieldByIndex(t, index, SYMTAB_PARAM_STATUS) ==
                        PARAM_IN_EVALUATION) {
                    ErrorInEvaluation = EV_Error;
                    ft_SetSemanticErrorForStatement(
                        (FortTree)((TableDescriptor)(d->parent_td))->FortTreePtr, 
                         expr,  ft_RECURSIVE_PARAMETER_DEFN);
		    value = 0; /* error case evaluates to zero */
                } else {
                    SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
                        PARAM_IN_EVALUATION);
                    value = evalConstIntExpr(d, 
                        SymGetFieldByIndex(t, index, SYMTAB_EXPR));
                    if (ErrorInEvaluation != EV_Error) {
                        SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
                            PARAM_VALUE_DEFINED);
                        SymPutFieldByIndex(t, index, SYMTAB_PARAM_VALUE, value);
                    } else { 
                        SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
                            PARAM_UNDEFINED);
                        value = 0; /* return value of zero on error */ 
                    }
                }
            }
        }
        return value;
    }
    case GEN_BINARY_PLUS:
        return  evalConstIntExpr(d, gen_BINARY_PLUS_get_rvalue1(expr)) + 
            evalConstIntExpr(d, gen_BINARY_PLUS_get_rvalue2(expr));

    case GEN_BINARY_MINUS:
        return  evalConstIntExpr(d, gen_BINARY_MINUS_get_rvalue1(expr)) -
            evalConstIntExpr(d, gen_BINARY_MINUS_get_rvalue2(expr));

    case GEN_BINARY_TIMES:
        return  evalConstIntExpr(d, gen_BINARY_TIMES_get_rvalue1(expr)) *
            evalConstIntExpr(d, gen_BINARY_TIMES_get_rvalue2(expr));

    case GEN_BINARY_DIVIDE:
    {
      int leftop, rightop;

      /*
       * Check for divide by zero. Signal the error and return 0 if we hit
       * this case (for lack of anything better to do).
       */
      leftop = evalConstIntExpr(d, gen_BINARY_DIVIDE_get_rvalue1(expr));
      rightop = evalConstIntExpr(d, gen_BINARY_DIVIDE_get_rvalue2(expr));
      if (!rightop) {
	/*
	 * Was there an error already? If so, then don't overwrite the
	 * existing error condition.
	 */
	if (ErrorInEvaluation != EV_Error)
	  ft_SetSemanticErrorForStatement(
            (FortTree)((TableDescriptor)(d->parent_td))->FortTreePtr, expr,
	    ft_BAD_TYPE);
	ErrorInEvaluation = EV_Error;
	return 0;		/* error case evaluates to zero */
      } else 
	return leftop / rightop;
    }

    case GEN_BINARY_EXPONENT:
    {
        int answer;
        int exponent = 
            evalConstIntExpr(d, gen_BINARY_EXPONENT_get_rvalue2(expr));

        value = evalConstIntExpr(d, gen_BINARY_EXPONENT_get_rvalue1(expr));

        /*  What should be returned for 0**0?  */
        /*  At the moment, we return 0.        */
        if ((value == 1) || (value == 0))
            answer = value;
        else if (exponent < 0)
            answer = 0;
        else
        {
            int bit_position = 1;
            int product = value;
            answer = 1;
            while (bit_position <= exponent)
            {
                if (exponent & bit_position)
                    answer = answer * product;
                product = product * product;
                bit_position <<= 1;
            }
        }
        return answer;
    }

    case GEN_UNARY_MINUS:
        return  - evalConstIntExpr(d, gen_UNARY_MINUS_get_rvalue(expr));

    default:
        ErrorInEvaluation = EV_Error;
        ft_SetSemanticErrorForStatement(
            (FortTree)((TableDescriptor)(d->parent_td))->FortTreePtr, 
             expr,  ft_UNEXPECTED_TYPE_IN_EXPR);
        return 0;
    }
}


/* 
 *    recursive evaluator for constant expressions
 *    sets "static int ErrorInEvaluation" to non-zero value if an
 *    error is encountered during evaluation
 *
 *    ASSUMPTIONS: each parameter defined in the symbol table has the
 *      symbol table field SYMTAB_EXPR defined to be the astnode 
 *      that is the head of the expression tree for the parameter's value 
 */
static int fastEvalConstIntExpr(SymDescriptor d, AST_INDEX expr)
{
  SymTable t = d->Table;
  int value;

  switch (gen_get_node_type(expr)) 
    {
    case GEN_CONSTANT:
      if (gen_get_real_type(expr) != TYPE_INTEGER) 
	{
	  ErrorInEvaluation = EV_Error;
	  return 0;
	}
      else 
	return atoi(gen_get_text(expr));

    case GEN_IDENTIFIER:
      {
	int index = SymQueryIndex(t, gen_get_text(expr));
	if (!IS_MANIFEST_CONSTANT(t, index)) 
	  {
	    ErrorInEvaluation = EV_Error;
	    return 0;
	  }
	else 
	  {
	    if (SymGetFieldByIndex(t, index, SYMTAB_PARAM_STATUS) == 
		PARAM_VALUE_DEFINED)
	      {
		value = SymGetFieldByIndex(t, index, SYMTAB_PARAM_VALUE);
	      }
	    else 
	      {
		if (SymGetFieldByIndex(t, index, SYMTAB_PARAM_STATUS) ==
		    PARAM_IN_EVALUATION) 
		  {
		    ErrorInEvaluation = EV_Error;
		    return 0;
		  }
		else 
		  {
		    SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
				       PARAM_IN_EVALUATION);
		    value = fastEvalConstIntExpr
		      (d, SymGetFieldByIndex(t, index, SYMTAB_EXPR));
		  
		    if (ErrorInEvaluation != EV_Error) 
		      {
			SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
					   PARAM_VALUE_DEFINED);
			SymPutFieldByIndex(t, index, SYMTAB_PARAM_VALUE, value);
		      }
		    else  
		      {
			SymPutFieldByIndex(t, index, SYMTAB_PARAM_STATUS, 
					   PARAM_UNDEFINED);
		      }
		  }
	      }
	  }
	return value;
      }

    case GEN_BINARY_PLUS:
      return  fastEvalConstIntExpr(d, gen_BINARY_PLUS_get_rvalue1(expr)) + 
	      fastEvalConstIntExpr(d, gen_BINARY_PLUS_get_rvalue2(expr));

    case GEN_BINARY_MINUS:
      return  fastEvalConstIntExpr(d, gen_BINARY_MINUS_get_rvalue1(expr)) -
	      fastEvalConstIntExpr(d, gen_BINARY_MINUS_get_rvalue2(expr));

    case GEN_BINARY_TIMES:
      return  fastEvalConstIntExpr(d, gen_BINARY_TIMES_get_rvalue1(expr)) *
	      fastEvalConstIntExpr(d, gen_BINARY_TIMES_get_rvalue2(expr));

    case GEN_BINARY_DIVIDE:
      {
	int leftop, rightop;

	/*
	 * Check for divide by zero. Signal the error and return 0 if we hit
	 * this case (for lack of anything better to do).
	 */
	leftop  = fastEvalConstIntExpr(d, gen_BINARY_DIVIDE_get_rvalue1(expr));
	rightop = fastEvalConstIntExpr(d, gen_BINARY_DIVIDE_get_rvalue2(expr));
	if (!rightop) 
	  {
	    /*
	     * Was there an error already? If so, then don't overwrite the
	     * existing error condition.
	     */
	    ErrorInEvaluation = EV_Error;
	    return 0;		/* error case evaluates to zero */
	  } 
	else 
	  return leftop / rightop;
      }

    case GEN_BINARY_EXPONENT:
      {
	int answer;
	int exponent = 
	  fastEvalConstIntExpr(d, gen_BINARY_EXPONENT_get_rvalue2(expr));

	value = fastEvalConstIntExpr(d, gen_BINARY_EXPONENT_get_rvalue1(expr));

	/*  What should be returned for 0**0?  */
	/*  At the moment, we return 0.        */
	if ((value == 1) || (value == 0))
	  answer = value;
	else if (exponent < 0)
	  answer = 0;
	else
	  {
	    int bit_position = 1;
	    int product = value;
	    answer = 1;
	    while (bit_position <= exponent)
	      {
		if (exponent & bit_position)
		  answer = answer * product;
		product = product * product;
		bit_position <<= 1;
	      }
	  }
	return answer;
      }

    case GEN_UNARY_MINUS:
      return  - fastEvalConstIntExpr(d, gen_UNARY_MINUS_get_rvalue(expr));

    default:
      ErrorInEvaluation = EV_Error;
      return 0;
    }
}
