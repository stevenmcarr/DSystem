/* $Id: expr3.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/gen.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>

#include <libs/f2i/call.h>

/* this file contains things removed from expression.c */
/* in order to speed up compilation ...		       */

/* forward declarations */




/* ILOC instructions associated with typed branches and subtractions */
/* (error, integer, float, double, complex, error)                   */
int subs_by_type[] = { ERR, iSUB, fSUB, dSUB, cSUB, qSUB, ERR};
int compares_by_type[] = { ERR, iCMP, fCMP, dCMP, cCMP, qCMP, ERR};




/* returns an array index for a given variable type     */
/* this is used to index arrays like "compares_by_type" */
int index_by_type( int type )
  // int type;	/* variable type */
{
  register int result;	/* returned index value */

  switch(type)
  {
    case TYPE_INTEGER:		result = 1;	break;
    case TYPE_REAL:		result = 2;	break;
    case TYPE_DOUBLE_PRECISION:	result = 3;	break;
    case TYPE_COMPLEX:		result = 4;	break;
    case TYPE_DOUBLE_COMPLEX:	result = 5;	break;
    default:			result = 6;	break;
  }

  return result;
} /* index_by_type */




/* generate iloc for a relational expression */
int relOp( AST_INDEX node, int comparator )
  // AST_INDEX	node;
  // int 		comparator;
{
  AST_INDEX 	lhs, rhs;
  int	lhs_reg, lhs_type;
  int	rhs_reg, rhs_type;
  int   result, result_type;
  int   compare_op;
  int   cc_reg;

  if (aiDebug > 1)
     (void) fprintf(stdout, "relOp( %d ).\n", node);

  generate(0, NOP, 0, 0, 0, "Relational expression");

  lhs = gen_BINARY_LT_get_rvalue1(node);
  rhs = gen_BINARY_LT_get_rvalue2(node);
  
  if (gen_get_real_type(lhs) == TYPE_CHARACTER ||
      gen_get_real_type(rhs) == TYPE_CHARACTER)
  {
    /* should be converted to intrinsic call */
    if (gen_get_real_type(lhs) == TYPE_CHARACTER &&
	gen_get_real_type(rhs) == TYPE_CHARACTER)
    {
      int reg1, reg2, len1, len2;
      
      /* start with the easy stuff */
      ERROR("relOp","Character compare being inlined.",WARNING);
      ERROR("relOp","This code is probably not correct.",WARNING);
      len1 = getStringLengthIntoReg(lhs);
      len2 = getStringLengthIntoReg(rhs);
      if (gen_get_node_type(lhs) == GEN_BINARY_CONCAT)
      {
	/* copy lhs onto stack */
	struct CharDesc Target[2],Source[MAX_CONCATS];
	int size = NewStringLength(lhs);
	int dummy = 0;
	
	createStackTarget(Target,size);
	evalCharExpr(lhs,Source,&dummy,MAX_CONCATS);
	generate_move_string(Target,Source);

	reg1 = Target[0].addr;
      }
      else
	reg1 = AddressFromNode(lhs);
      if (gen_get_node_type(rhs) == GEN_BINARY_CONCAT)
      {
	/* copy lhs onto stack */
	struct CharDesc Target[2],Source[MAX_CONCATS];
	int size = NewStringLength(rhs);
	int dummy = 0;
	
	createStackTarget(Target,size);
	evalCharExpr(lhs,Source,&dummy,MAX_CONCATS);
	generate_move_string(Target,Source);

	reg2 = Target[0].addr;
      }
      else
	reg2 = AddressFromNode(rhs);
	return(generate_char_compare(reg1,reg2,len1,len2,comparator));
    }
    else 
      ERROR("relOp","Cannot handle lexical compares",FATAL);
  }

  /* generate code for cases where character types are not involved */
  lhs_reg = getExprInReg(lhs);
  rhs_reg = getExprInReg(rhs);

  lhs_type = fst_my_GetFieldByIndex(ft_SymTable, lhs_reg, SYMTAB_TYPE);
  rhs_type = fst_my_GetFieldByIndex(ft_SymTable, rhs_reg, SYMTAB_TYPE);
  
  /* insure that the types are consistent */
  if (lhs_type == rhs_type)
  {
  	result_type = lhs_type;
  }

  else
  {
    result_type = Table2(lhs_type, rhs_type);

    if (lhs_type != result_type)
       lhs_reg = getConversion(lhs_reg, result_type);
    if (rhs_type != result_type)
       rhs_reg = getConversion(rhs_reg, result_type);
  }

  /* create a register to hold the logical result of the compare */
  compare_op = compares_by_type[index_by_type(result_type)];
  result  = TempReg(lhs_reg, rhs_reg, compare_op, TYPE_LOGICAL);

  /* generate compare statement by type */
  generate(0, compare_op, lhs_reg, rhs_reg, result, NOCOMMENT);

  /* generate the logical result from the result of the compare */
  cc_reg = TempReg(result, 0, comparator, TYPE_LOGICAL);
  generate(0, comparator, result, cc_reg, 0, NOCOMMENT);

  return cc_reg;
} /* relOp */




/* generate iloc for a boolean operation */
int booleanOp( AST_INDEX node )
  // AST_INDEX	node;
{
  AST_INDEX	lhs, rhs;
  register int	lhs_reg, rhs_reg, lhs_type, rhs_type, type, result;
  int 		one, True;

  lhs = gen_BINARY_AND_get_rvalue1(node);
  rhs = gen_BINARY_AND_get_rvalue2(node);

  lhs_reg = getExprInReg(lhs);
  rhs_reg = getExprInReg(rhs);

  lhs_type = fst_my_GetFieldByIndex(ft_SymTable, lhs_reg, SYMTAB_TYPE);
  rhs_type = fst_my_GetFieldByIndex(ft_SymTable, rhs_reg, SYMTAB_TYPE);

  if (lhs_type != TYPE_LOGICAL)
  {
    (void) sprintf(error_buffer, "lhs of boolean operator has type '%s'", 
	    TypeName(lhs_type));
    ERROR("HandleBoolean", error_buffer, WARNING);
  }

  if (rhs_type != TYPE_LOGICAL)
  {
    (void) sprintf(error_buffer, "rhs of boolean operator has type '%s'",
	    TypeName(rhs_type) );
    ERROR("HandleBoolean", error_buffer, WARNING);
  }

  type = gen_get_node_type(node);
  switch(type)
  {
    case GEN_BINARY_AND:
	result = TempReg(lhs_reg, rhs_reg, lAND, TYPE_LOGICAL);
	generate(0, lAND, lhs_reg, rhs_reg, result, NOCOMMENT);
	break;

    case GEN_BINARY_OR:
	result = TempReg(lhs_reg, rhs_reg, lOR, TYPE_LOGICAL);
	generate(0, lOR, lhs_reg, rhs_reg, result, NOCOMMENT);
	break;

    case GEN_BINARY_EQV:
	one = TempReg(lhs_reg, rhs_reg, lXOR, TYPE_LOGICAL);
	generate(0, lXOR, lhs_reg, rhs_reg, one, "EQV");
	True   = getIntConstantInRegister("1");
	result = TempReg(True, one, lXOR, TYPE_LOGICAL);
	generate(0, lXOR, True, one, result, NOCOMMENT);
	break;

    case GEN_BINARY_NEQV:
	/* since args are just logicals, lXOR suffices */
	result  = TempReg(lhs_reg, rhs_reg, lXOR, TYPE_LOGICAL);
	generate(0, lXOR, lhs_reg, rhs_reg, result, "NEQV");
	break;

    default:
	(void) sprintf(error_buffer, "unknown operator %d", type);
	ERROR("HandleBoolean", error_buffer, WARNING);
	break;
  }
  return result;
} /* booleanOp */




/* generate iloc for a unary NOT */
int HandleUnaryNot(AST_INDEX node)
  // AST_INDEX node;
{
  int reg, expr, True;
  AST_INDEX son;

  son = gen_UNARY_NOT_get_rvalue(node);
  expr = getExprInReg(son);
  if (fst_my_GetFieldByIndex(ft_SymTable, expr, SYMTAB_TYPE) != TYPE_LOGICAL)
     expr = getConversion(expr, TYPE_LOGICAL);

  True = getIntConstantInRegister("1");
  reg = TempReg(expr, True, lXOR, TYPE_LOGICAL);
  generate(0, lXOR, expr, True, reg, NOCOMMENT);

  return reg;
} /* HandleUnaryNot */
