/* $Id: cfgval_ops.C,v 1.2 1997/03/11 14:35:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <include/frontEnd/astnode.h>

ValOpType cfgval_op_tree2val(int treeOp)
{
    switch(treeOp)
    {
      case GEN_BINARY_EXPONENT:
	return VAL_OP_EXP;
      case GEN_BINARY_TIMES:
	return VAL_OP_TIMES;
      case GEN_BINARY_DIVIDE:
	return VAL_OP_DIVIDE;
      case GEN_BINARY_PLUS:
	return VAL_OP_PLUS;
      case GEN_BINARY_MINUS:
	return VAL_OP_MINUS;
      case GEN_BINARY_AND:
	return VAL_OP_AND;
      case GEN_BINARY_OR:
	return VAL_OP_OR;
      case GEN_BINARY_EQ:
	return VAL_OP_EQ;
      case GEN_BINARY_NE:
	return VAL_OP_NE;
      case GEN_BINARY_GE:
	return VAL_OP_GE;
      case GEN_BINARY_GT:
	return VAL_OP_GT;
      case GEN_BINARY_LE:
	return VAL_OP_LE;
      case GEN_BINARY_LT:
	return VAL_OP_LT;
      case GEN_BINARY_EQV:
	return VAL_OP_EQ;
      case GEN_BINARY_NEQV:
	return VAL_OP_NE;
      case GEN_BINARY_CONCAT:
	return VAL_OP_CONC;
      case GEN_UNARY_MINUS:
	return VAL_OP_MINUS;
      case GEN_UNARY_NOT:
	return VAL_OP_NOT;
      default:
	return (ValOpType) VAL_BOT_TYPE;
    }
}

int cfgval_op_val2tree(ValOpType valOp)
{
    switch(valOp)
    {
      case VAL_OP_EXP:
	return GEN_BINARY_EXPONENT;
      case VAL_OP_TIMES:
	return GEN_BINARY_TIMES;
      case VAL_OP_DIVIDE:
	return GEN_BINARY_DIVIDE;
      case VAL_OP_PLUS:
	return GEN_BINARY_PLUS;
      case VAL_OP_AND:
	return GEN_BINARY_AND;
      case VAL_OP_OR:
	return GEN_BINARY_OR;
      case VAL_OP_EQ:
	return GEN_BINARY_EQ;	// may be EQV instead -- beware!
      case VAL_OP_NE:
	return GEN_BINARY_NE;
      case VAL_OP_GE:
	return GEN_BINARY_GE;
      case VAL_OP_GT:
	return GEN_BINARY_GT;
      case VAL_OP_LE:
	return GEN_BINARY_LE;
      case VAL_OP_LT:
	return GEN_BINARY_LT;
      case VAL_OP_NOT:
	return GEN_UNARY_NOT;
	//
	// VAL_OP_MINUS should never be built into a value number
	// other things don't correspond to tree operations
      default:
	return GEN_ERROR;
    }
}
