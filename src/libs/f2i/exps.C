/* $Id: exps.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
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
#include <libs/f2i/call.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>

/* forward declarations */

/*  move bit_masks[] to data.c and make static  */
long
int bit_masks[] = {	0x00000001, 0x00000002, 0x00000004, 0x00000008,
			0x00000010, 0x00000020, 0x00000040, 0x00000080,
			0x00000100, 0x00000200, 0x00000400, 0x00000800,
			0x00001000, 0x00002000, 0x00004000, 0x00008000,
			0x00010000, 0x00020000, 0x00040000, 0x00080000,
			0x00100000, 0x00200000, 0x00400000, 0x00800000,
			0x01000000, 0x02000000, 0x04000000, 0x08000000,
			0x10000000, 0x20000000, 0x40000000, 0x80000000 };




/* variables and definitions needed to determine the opcode to be */
/* used for exponentiation and the correct types of the arguments */

#define	INVALID	0
#define	POW_II	1
#define	POW_RI	2
#define	POW_DI	3
#define	POW_CI	4
#define POW_QI	5
#define	POW_RR	6
#define	POW_DD	7
#define POW_CC	8
#define	POW_QQ	9

static int 	opcodes[]   = { ERR, iPOW, fPOWi, dPOWi, cPOWi,
			        qPOWi, fPOW, dPOW, cPOW, qPOW};
			        
static int	arg_types[] = {	INVALID, TYPE_INTEGER, TYPE_REAL, 
				TYPE_DOUBLE_PRECISION, TYPE_COMPLEX,
				TYPE_DOUBLE_COMPLEX, TYPE_REAL,
				TYPE_DOUBLE_PRECISION, TYPE_COMPLEX,
				TYPE_DOUBLE_COMPLEX};
				
static int	exp_types[] = {	INVALID, TYPE_INTEGER, TYPE_INTEGER,
				TYPE_INTEGER, TYPE_INTEGER, TYPE_INTEGER,
				TYPE_REAL, TYPE_DOUBLE_PRECISION,
				TYPE_COMPLEX, TYPE_DOUBLE_COMPLEX};
				
static int	PowTable[7][7]  =
			{{INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID},
			 {INVALID, POW_II,  POW_RI,  POW_DI,  POW_CI,  POW_QI,  INVALID},
			 {INVALID, POW_RR,  POW_RR,  POW_DD,  POW_CC,  POW_QQ,  INVALID},
			 {INVALID, POW_DD,  POW_DD,  POW_DD,  POW_QQ,  POW_QQ,  INVALID},
			 {INVALID, POW_CC,  POW_CC,  POW_QQ,  POW_CC,  POW_QQ,  INVALID},
			 {INVALID, POW_QQ,  POW_QQ,  POW_QQ,  POW_QQ,  POW_QQ,  INVALID},
			 {INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID}};




/* generate iloc for exponentiation, i.e. x ** y */
int HandleExponent( AST_INDEX node )
  //AST_INDEX	node;
{
  AST_INDEX	arg, exp;
  int		arg_reg, exp_reg, op, result;
  int		arg_type, exp_type, routine_type;

  arg = gen_BINARY_EXPONENT_get_rvalue1(node);
  exp = gen_BINARY_EXPONENT_get_rvalue2(node);

  arg_type = ATypeToIType(gen_get_converted_type(arg));
  arg_reg = getExprInReg(arg);

  exp_type = ATypeToIType(gen_get_converted_type(exp));
  exp_reg  = getExprInReg(exp);

  /*  Find the appropriate opcode as specified in PowTable  */
  routine_type = PowTable[index_by_type(exp_type)][index_by_type(arg_type)];

  if (routine_type != INVALID)
    {
      if (arg_type != arg_types[routine_type])
        arg_reg = getConversion(arg_reg, arg_types[routine_type]);
        
      if (exp_type != exp_types[routine_type])
        exp_reg = getConversion(exp_reg, exp_types[routine_type]);

      op = opcodes[routine_type];

      result = TempReg(arg_reg, exp_reg, op, arg_types[routine_type]);

      generate(0, op, arg_reg, exp_reg, result, "exponentiation");
    }

  else
    {
      (void) sprintf(error_buffer,
	"%s ** %s is an invalid expression.\n'%s'is of type %s, '%s' is of type %s.\n",
	gen_get_text(arg), gen_get_text(exp), gen_get_text(arg), TypeName(arg_type),
	gen_get_text(exp), TypeName(exp_type));
      ERROR("HandleExponent", error_buffer, FATAL);
    }

  return result;
} /* HandleExponent */
