/* $Id: intrins.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/include/gi.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>
#include <include/frontEnd/astnode.h>

#include <libs/f2i/mnemonics.h>




/* forward declaration */

static char buffer[256];

/* generate iloc for an invocation of an intrinsic function */
int HandleIntrinsic ( AST_INDEX invocation )

  //   AST_INDEX	invocation;    /* AST intrinsic invocation node */
  {
    AST_INDEX	list,
		parm_list;
    int		reg1,
		reg2,
		type1,
		type2,
		maxparms,
		index, 
		convert,
		code,
		opcode,
		creg1,
		creg2,
		treg,
                len1,
                len2,
		routine_type;
    STR_TEXT	routine_name;

  /*  get the routine name  */
    routine_name = gen_get_text(gen_INVOCATION_get_name(invocation));
    routine_type = gen_get_real_type(invocation);

  /*  is this a valid intrinsic?  */
    opcode = iloc_intrinsic (routine_name);
    if (opcode==A2I_INVALID_OPCODE) return SYM_INVALID_INDEX;	/*  not an intrinsic  */

  /*  Is this already declared as something other than intrinsic?  */
    index = fst_my_QueryIndex(ft_SymTable, routine_name);
    if ((index != SYM_INVALID_INDEX) && 
	(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_STORAGE_CLASS) & SC_EXTERNAL))
	return SYM_INVALID_INDEX;	/*  external function, not an intrinsic  */

  /*  find the parameter list  */
    parm_list = gen_INVOCATION_get_actual_arg_LIST(invocation);

  /*  count the parameters in the parameter list  */
    list = list_first(parm_list);
    maxparms = 0;

    while (list != ast_null_node)
      {
	list = list_next(list);
	maxparms++;
      }

  /*  process the generic  */
    switch(maxparms)
      {
	case 0:  /* no parameters provided for the intrinsic */
	
	  (void) sprintf(buffer, "no parameters specified for %s", routine_name);
	  ERROR("HandleIntrinsic", buffer, FATAL);
	  break;

	case 1:  /* one parameter provided for the intrinsic */

	  /* if it is not of type character, load the parameter into a register */
	  list = list_first(parm_list);
	  if (gen_get_real_type(list) != TYPE_CHARACTER)
	  {
	    reg1 = getExprInReg(list);
	    type1 = fst_my_GetFieldByIndex(ft_SymTable, reg1, SYMTAB_TYPE);
	    index = StrTempReg(routine_name, reg1, routine_type);
	  }

	/*  generate iloc instruction for appropriate intrinsic  */
	    switch (opcode)
	      {
		case fTRUNC:	/*  truncation:  float or generic  */
		case fROUND:	/*  nearest whole number:  float or generic */
		case fNINT:	/*  nearest integer:  float or generic  */
		case fLOG10:	/*  common logarithm:  generic  */
		case fTAN:	/*  tangent:  float or generic  */
		case fASIN:	/*  arcsine:  float or generic  */
		case fACOS:	/*  arccosine:  float or generic  */
		case fATAN:	/*  arctangent (1 arg):  float or generic  */
		case fSINH:	/*  hyperbolic sine:  float or generic  */
		case fCOSH:	/*  hyperbolic cosine:  float or generic  */
		case fTANH:	/*  hyperbolic tangent:  float or generic  */
		  if (type1 == TYPE_REAL)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode+1, reg1, index, 0, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case fABS:	/*  absolute value:  float or generic  */
		  switch (type1)
		    {
		      case TYPE_REAL:
		     	generate(0, opcode, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_INTEGER:
			generate(0, opcode-1, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_DOUBLE_PRECISION:
			generate(0, opcode+1, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_COMPLEX:
			generate(0, opcode+2, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_DOUBLE_COMPLEX:
			generate(0, opcode+3, reg1, index, 0, "invoke intrinsic");
			break;
		      default:
		     	(void) sprintf(buffer, "%s received an argument of invalid type (%s)",
				routine_name, TypeName(type1)); 
		     	ERROR("HandleIntrinsic", buffer, FATAL);
			break;
		    };
		  break;

		case fSQRT:	/*  square root:  float or generic  */
		case fEXP:	/*  exponential:  float or generic  */
		case fLOG:	/*  natural logarithm:  generic  */
		case fSIN:	/*  sine:  float or generic  */
		case fCOS:	/*  cosine:  float or generic  */
		  switch (type1)
		    {
		      case TYPE_REAL:
			generate(0, opcode, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_DOUBLE_PRECISION:
			generate(0, opcode+1, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_COMPLEX:
			generate(0, opcode+2, reg1, index, 0, "invoke intrinsic");
			break;
		      case TYPE_DOUBLE_COMPLEX:
			generate(0, opcode+3, reg1, index, 0, "invoke intrinsic");
			break;
		      default:
		     	(void) sprintf(buffer, "%s received an argument of invalid type (%s)",
				routine_name, TypeName(type1)); 
		     	ERROR("HandleIntrinsic", buffer, FATAL);
			break;
		    };
		  break;

		case qIMAG:     /*  imaginary part of a double complex argument */
	        case qCONJ:     /*  conjugate of a double complex argument */
		  if (type1 == TYPE_DOUBLE_COMPLEX)
		    generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else if (type1 == TYPE_COMPLEX)
		    {
		      reg1 = getConversion(reg1, TYPE_DOUBLE_COMPLEX);
		      generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case cIMAG:	/*  imaginary part of complex argument  */
		case cCONJ:	/*  conjugate of a complex argument  */
		  if (type1 == TYPE_COMPLEX)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_COMPLEX)
		    {
		      reg1 = getConversion(reg1, TYPE_COMPLEX);
		      generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case LEN:	/*  length of string  */
		  /* since this might get a little long, and is
		   * different, we handle it in another function
		   */
		  if (gen_get_real_type(list) == TYPE_CHARACTER) {
		    if (aiAnnotate)
		      generate(0, NOP, 0, 0, 0, "simulate len intrinsic");
		    reg1 = generate_len(invocation);
		    index=StrTempReg(routine_name, reg1, routine_type);
		    generate_move(index, reg1, TYPE_INTEGER);
		  }
		  else {
		    (void) sprintf(buffer, "%s receive an argument of invalid type (%s)",
				   routine_name, TypeName(type1));
		    ERROR("HandleIntrinsic", buffer, FATAL);
		  }
		  break;

		case iABS:	/*  absolute value:  integer  */
		case lNOT:	/*  complement:  integer  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case d2i:	/*  convert to integer:  double precision  */
		case d2f:	/*  convert to real:  double precision  */
		case dTRUNC:	/*  truncation:  double precision  */
		case dROUND:	/*  nearest whole number:  double precision  */
		case dNINT:	/*  nearest integer:  double precision  */
		case dABS:	/*  absolute value:  double precision  */
		case dSQRT:	/*  square root:  double precision  */
		case dEXP:	/*  exponential:  double precision  */
		case dLOG:	/*  natural logarithm:  double precision  */
		case dLOG10:	/*  common logarithm:  double precision  */
		case dSIN:	/*  sine:  double precision  */
		case dCOS:	/*  cosine:  double precision  */
		case dTAN:	/*  tangent:  double precision  */
		case dASIN:	/*  arcsine:  double precision  */
		case dACOS:	/*  arccosine:  double precision  */
		case dATAN:	/*  arctangent (1 arg):  double precision  */
		case dSINH:	/*  hyperbolic sine:  double precision  */
		case dCOSH:	/*  hyperbolic cosine:  double precision  */
		case dTANH:	/*  hyperbolic tangent:  double precision  */
		  if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case cABS:	/*  absolute value:  complex  */
		case cSQRT:	/*  square root:  complex  */
		case cEXP:	/*  exponential:  complex  */
		case cLOG:	/*  natural logarithm: complex  */
		case cSIN:	/*  sine:  complex  */
		case cCOS:	/*  cosine:  complex  */
		  if (type1 == TYPE_COMPLEX)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case f2i:	/*  convert to integer:  generic  */
		case i2f:	/*  convert to real:  generic  */
		case f2d:	/*  convert to double precision:  generic  */
		  if (type1 != routine_type)
		     generate_move(index, getConversion(reg1, routine_type), 
				routine_type);
		  else
		     generate_move(index, reg1, routine_type);
		  break;

		case i2d:       /* convert to double: integer */
		  if (type1 == TYPE_INTEGER)
		    generate_move(index, getConversion(reg1, TYPE_DOUBLE_PRECISION),
				  TYPE_DOUBLE_PRECISION);
		  else
		    {
		      (void) sprintf(buffer, "dfloat received an argument of invalid type (%s)",
			 TypeName(type1));
		      ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case i2c:	/*  convert to complex:  generic  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode, reg1, index, 0, "invoke intrinsic");
		  else if (type1 == TYPE_REAL)
		     generate(0, opcode+5, reg1, index, 0, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode+10, reg1, index, 0, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case CHAR:	/*  convert to character:  integer  */
		  if (type1 == TYPE_INTEGER)
		    generate_move(index,reg1,TYPE_INTEGER);
		  else
		    {
		     (void) sprintf(buffer, "%s received an argument of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;
		  
		case iCHAR:	/*  convert to integer:  character  */
		  while (gen_get_node_type(list) == GEN_BINARY_CONCAT) 
		    list = gen_BINARY_CONCAT_get_rvalue1(list);
		  reg1 = AddressFromNode(list);
		  index = StrTempReg(routine_name, reg1, routine_type);
		  generate_load(index, reg1, TYPE_CHARACTER, -1, "&unknown");
		  break;

		default:	/*  too few arguments specified  */
		  (void) sprintf(buffer, "only one parameter specified for %s", 
			routine_name);
	  	  ERROR("HandleIntrinsic", buffer, FATAL);
		  break;
	      }
	  break;

	case 2:

	  /* evaluate parameters                                              */
	  /* load the address of character expressions -- since it can't be   */
	  /* a concat, we can take the address.  Also, need to more registers */
	  /* to hold the lengths of the characters.                           */

	  list = list_first(parm_list);
	  if (gen_get_real_type(list) == TYPE_CHARACTER) 
	    {
	      reg1 = AddressFromNode(list);
	      len1 = getStringLengthIntoReg(list);
	      type1 = TYPE_CHARACTER;
	    }
	  else
	    {
	      reg1 = getExprInReg(list);
	      type1 = fst_my_GetFieldByIndex(ft_SymTable, reg1, SYMTAB_TYPE);
	    }
	  list = list_next(list);
	  if (gen_get_real_type(list) == TYPE_CHARACTER) 
	    {
	      reg2 = AddressFromNode(list);
	      len2 = getStringLengthIntoReg(list);
	      type2 = TYPE_CHARACTER;
	    }
	  else
	    {
	      reg2 = getExprInReg(list);
	      type2 =  fst_my_GetFieldByIndex(ft_SymTable, reg2, SYMTAB_TYPE);
            }
	      
	  /* insure that both expressions are of the same type */
	  if (type1 != type2)
	    {
	      (void) sprintf(buffer, "%s received arguments of different types (%s and %s)",
			routine_name, TypeName(type1), TypeName(type2));
	      ERROR("HandleIntrinsic", buffer, FATAL);
	    }

	  /* create register for the result */
	  (void) sprintf(buffer, "%s[%d,%d]", routine_name, reg1, reg2);
	  index = SymInsertSymbol(buffer, routine_type, 
		OC_IS_DATA, 0, SC_NO_MEMORY, NO_ALIAS);

	/*  perform the appropriate instruction  */
	    switch (opcode)
	      {
		case iMOD:	/*  remaindering:  integer or generic  */
		case iMAX:	/*  maximum:  generic  */
		case iMIN:	/*  minimum:  generic  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else if (type1 == TYPE_REAL)
		     generate(0, opcode+1, reg1, reg2, index, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode+2, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case fSIGN:	/*  transfer of sign:  float or generic  */
		case fDIM:	/*  positive difference:  float or generic  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode-1, reg1, reg2, index, "invoke intrinsic");
		  else if (type1 == TYPE_REAL)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode+1, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case fATAN2:	/*  arctangent (2 args):  float or generic  */
		  if (type1 == TYPE_REAL)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode+1, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case iSIGN:	/*  transfer of sign:  integer  */
		case iDIM:	/*  positive difference:  integer  */
		case lAND:	/*  logical and:  integer  */
		case lOR:	/*  logical or:  integer  */
		case lXOR:	/*  logical exclusive or:  integer  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case lSHIFT:	/*  logical shift:  integer  */
		  if (type1 == TYPE_INTEGER)
		     generate(0, opcode, reg2, reg1, index, "invoke ISHFT");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case AMAX0:	/*  maximum:  integer to float  */
		  if (type1 == TYPE_INTEGER)
		    {
		     treg = TempReg(reg1, reg2, iMAX, TYPE_INTEGER);
		     generate(0, iMAX, reg1, reg2, treg, "invoke intrinsic");
		     generate_move(index, getConversion(treg, TYPE_REAL), TYPE_REAL);
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case MAX1:	/*  maximum:  float to integer  */
		  if (type1 == TYPE_REAL)
		    {
		     treg = TempReg(reg1, reg2, fMAX, TYPE_REAL);
		     generate(0, fMAX, reg1, reg2, treg, "invoke intrinsic");
		     generate_move(index, getConversion(treg, TYPE_INTEGER), TYPE_INTEGER);
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case AMIN0:	/*  minimum:  integer to float  */
		  if (type1 == TYPE_INTEGER)
		    {
		     treg = TempReg(reg1, reg2, iMIN, TYPE_INTEGER);
		     generate(0, iMIN, reg1, reg2, treg, "invoke intrinsic");
		     generate_move(index, getConversion(treg, TYPE_REAL), TYPE_REAL);
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case MIN1:	/*  minimum:  float to integer  */
		  if (type1 == TYPE_REAL)
		    {
		     treg = TempReg(reg1, reg2, fMIN, TYPE_REAL);
		     generate(0, fMIN, reg1, reg2, treg, "invoke intrinsic");
		     generate_move(index, getConversion(treg, TYPE_INTEGER), TYPE_INTEGER);
		    }
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case fMOD:	/*  remaindering:  float  */
		case dPROD:	/*  double precision product  */
		case fMAX:	/*  maximum:  float  */
		case fMIN:	/*  minimum:  float  */
		  if (type1 == TYPE_REAL)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case dMOD:	/*  remaindering:  double precision  */
		case dSIGN:	/*  transfer of sign:  double precision  */
		case dDIM:	/*  positive difference:  double precision  */
		case dMAX:	/*  maximum:  double precision  */
		case dMIN:	/*  minimum:  double precision  */
		case dATAN2:	/*  arctangent (2 args):  double precision  */
		  if (type1 == TYPE_DOUBLE_PRECISION)
		     generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		     (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			routine_name, TypeName(type1)); 
		     ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case i2c:	/*  convert to complex:  generic  */
		  generate(0, COMMENT, 0, 0, 0, "Here comes a CMPLX conversion");
		  if (type1 == TYPE_REAL)  /* real part */
		    creg1 = reg1;
		  else
		    creg1 = getConversion(reg1, TYPE_REAL);

		  if (type2 == TYPE_REAL)  /* imaginary part */
		    creg2 = reg2;
		  else
		    creg2 = getConversion(reg2, TYPE_REAL);

		  reg1 = TempReg(creg1, creg2, cCOMPLEX, TYPE_COMPLEX);
		  generate(0, cCOMPLEX, creg1, creg2, reg1, "Type Coercion");
		  generate(0, c2c, reg1, index, 0, "For Partial!");
		  break;

		case qCOMPLEX:  /*  double precision to complex conversion  */
		  if (type1 == TYPE_DOUBLE_PRECISION)
		    generate(0, opcode, reg1, reg2, index, "invoke intrinsic");
		  else
		    {
		      (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
			 routine_name, TypeName(type1));
		      ERROR("HandleIntrinsic", buffer, FATAL);
		    }
		  break;

		case INDEX:	/*  index in the string  */
		  generate_index(reg1, reg2, len1, len2, index);
		  break;
		  
		case LGE:	/*  character comparison (GE)  */
		case LGT:	/*  character comparison (GT)  */
		case LLE:	/*  character comparison (LE)  */
		case LLT:	/*  character comparison (LT)  */
		  (void) sprintf(buffer, "%s is not yet implemented", routine_name);
		  ERROR("HandleIntrinsic", buffer, FATAL);
		  break;

		default:	/*  too many arguments specified  */
		  (void) sprintf(buffer, "two parameters specified for %s", 
			routine_name);
	  	  ERROR("HandleIntrinsic", buffer, FATAL);
		  break;
	      }
	  break;

	default:
	  /* evaluate the first two parameters */
	  list = list_first(parm_list);
	  reg1 = getExprInReg(list);
	  type1 = fst_my_GetFieldByIndex(ft_SymTable, reg1, SYMTAB_TYPE);
	  list = list_next(list);
	  reg2 = getExprInReg(list);
	  type2 = fst_my_GetFieldByIndex(ft_SymTable, reg2, SYMTAB_TYPE);
	  list = list_next(list);

	  /* insure that both expressions are of the same type */
	  if (type1 != type2)
	    {
	      (void) sprintf(buffer, "%s received arguments of different types (%s and %s)",
			routine_name, TypeName(type1), TypeName(type2));
	      ERROR("HandleIntrinsic", buffer, FATAL);
	    }

	  /* create register for the result */
	  (void) sprintf(buffer, "%s[%d]", routine_name, parm_list);
	  index = SymInsertSymbol(buffer, routine_type, 
		OC_IS_DATA, 0, SC_NO_MEMORY, NO_ALIAS);

	  convert = TYPE_UNKNOWN;
	  /* select the appropriate opcode */
	  switch (opcode)
	    {
	      case iMAX:	/*  maximum:  generic  */
	      case iMIN:	/*  minimum:  generic  */
		if (type1 == TYPE_INTEGER)
		   code = opcode;
		else if (type1 == TYPE_REAL)
		   code = opcode + 1;
		else if (type1 == TYPE_DOUBLE_PRECISION)
		   code = opcode + 2;
		else
		  {
		   (void) sprintf(buffer, "%s received arguments of invalid type (%s)",
		     routine_name, TypeName(type1)); 
		   ERROR("HandleIntrinsic", buffer, FATAL);
		  }
		break;

	      case AMAX0:	/*  maximum:  integer to float  */
		convert = TYPE_REAL;
		code = iMAX;
		break;

	      case AMIN0:	/*  minimum:  integer to float  */
		convert = TYPE_REAL;
		code = iMIN;
		break;

	      case MAX1:	/*  maximum:  float to integer  */
		convert = TYPE_INTEGER;
		code = fMAX;
		break;

	      case MIN1:	/*  minimum:  float to integer  */
		convert = TYPE_INTEGER;
		code = fMIN;
		break;

	      case fMAX:	/*  maximum:  float  */
	      case fMIN:	/*  minimum:  float  */
	      case dMAX:	/*  maximum:  double precision  */
	      case dMIN:	/*  minimum:  double precision  */
		code = opcode;
		break;

	      default:	/*  too many arguments specified  */
		(void) sprintf(buffer, "too many parameters specified for %s", 
			routine_name);
	  	ERROR("HandleIntrinsic", buffer, FATAL);
		break;
	    }

	  /* process the first two parameters */
	 creg1 = TempReg(reg1, reg2, code, type1);
	 generate(0, code, reg1, reg2, creg1, 
		  "first two intrinsic parameters");

	  /* process the remaining parameters */
	  while (list != ast_null_node)
	    {
	      reg2 = getExprInReg(list);
	      type2 =  fst_my_GetFieldByIndex(ft_SymTable, reg2, SYMTAB_TYPE);

	      /* insure that both expressions are of the same type */
	      if (type1 != type2)
		{
		  (void) sprintf(buffer, "%s received arguments of different types (%s and %s)",
			  routine_name, TypeName(type1), TypeName(type2));
		  ERROR("HandleIntrinsic", buffer, FATAL);
		}

	      creg2 = TempReg(creg1, reg2, code, type2);
	      generate(0, code, creg1, reg2, creg2, 
		       "remaining intrinsic parameters");
	      creg1 = creg2;

	      list = list_next(list);
	    }

	  /* convert the result, if necessary */
	  if (convert != TYPE_UNKNOWN)
	    index = getConversion(creg2, convert);
	  else
	    generate_move(index, creg2, type1);

	  break;
	}

    return index;
  } /* HandleIntrinsic */




/* generate iloc for the FORTRAN intrinsic LEN */
int generate_len(AST_INDEX invocation)
  // AST_INDEX invocation;
{
  struct CharDesc charDesc[MAX_CONCATS];
  int num_expr = 0;
  AST_INDEX parm_list = gen_INVOCATION_get_actual_arg_LIST(invocation);
  AST_INDEX first_parm = list_first(parm_list);
  int i;
  int temp; 
  int zero = getIntConstantInRegister("0");
  int sum = StrTempReg("sum", (int) invocation, TYPE_INTEGER);
  int tempsum = StrTempReg("sum", sum, TYPE_INTEGER);

  evalCharExpr(first_parm, charDesc, &num_expr, MAX_CONCATS);

  generate_move(sum, zero, TYPE_INTEGER);
  for (i=0; i<num_expr; i++) {
    /* keep a running total of the sum */
    switch (charDesc[i].description) {
    case CHAR_NORMAL:
      temp = getConstantInRegFromInt(charDesc[i].misc);
      break;
    case CHAR_FUNCTION:
      /* 
       * temp = ???
       * should I call function???
       */
      break;
    case CHAR_UNKNOWN_LEN:
      temp = charDesc[i].misc;
      break;
    default:
      ERROR("generate_len","Character description type unknown",FATAL);
      break;
    }
    tempsum = TempReg(sum, temp, iADD, TYPE_INTEGER);
    generate(0, iADD, sum, temp, tempsum, NOCOMMENT);
    generate_move(sum, tempsum, TYPE_INTEGER);
  }
  return(sum);
} /* generate_len */
  
