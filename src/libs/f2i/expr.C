/* $Id: expr.C,v 1.8 2000/01/27 19:43:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*
 *  Expression code - 
 *
 */

#include <libs/support/misc/general.h>
#include <sys/file.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>

#include <libs/f2i/mnemonics.h>

#include <libs/Memoria/include/ASTToIntMap.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>

/* functions in this file */
/* int getExprInReg(node)          get expr at node (node) into a register */
/*                                 and returns register value, makes iloc  */
/*                                 warning: does not work well w/ strings  */
/* int getConversion( reg, type )  converts register to type, makes iloc   */
/* int getIdInRegister(node)       gEIR for identifier, substring,         */
/*                                 and subscript                           */
/* int getConstantRegister( node ) gEIR for constants                      */
/* int getConstantInRegFromInt(val) gEIR for (int) constant                */
/*                                 CAVEAT:  must be const at compile time  */
/* int getConstantInRegFromString( name, type, converted_type )                            */
/*                                 gEIR for named (char *) const.          */
/* int getConstantInRegFromIndex( Index, converted_type )                                  */
/*                                 gEIR for const. in ai_SymTable          */
/* int binaryOp( node )            generates binary arithmetic operation   */
/* int ArithOp( op, type )         returns correctly typed arithmetic op   */
/* int Table2( type1, type2 )      determines result type for operation    */
/* 

/* global data structure */
extern int subs_by_type[];

extern ASTToIntMap *ASTRegMap;

/* local data structures */
static char name_buffer[128];

/* forward declarations */




/* given an AST node, generate iloc to put the value of */
/* the expression in a register and return the register */
int getExprInReg( AST_INDEX node )
  // AST_INDEX node;
{
  int Index, type, node_type, zero, temp, temp_type;

  if (aiDebug > 1)
     (void) fprintf(stdout, "getExprInReg( %d ).\n", node);

  /* change 6/24/91 */
  /* cannot load a string into a register */
  if (gen_get_real_type(node) == TYPE_CHARACTER)
    ERROR("getExprInReg", "Cannot load CHARACTER expression into register", SERIOUS);

  node_type = gen_get_node_type(node);

  switch(node_type)
  {
    case GEN_CONSTANT:
    case GEN_COMPLEX_CONSTANT:
	Index = getConstantRegister( node );
	break;

    case GEN_IDENTIFIER:
    case GEN_SUBSTRING:
    case GEN_SUBSCRIPT:
	Index = getIdInRegister( node );
	break;

    case GEN_BINARY_TIMES:
    case GEN_BINARY_PLUS:
    case GEN_BINARY_MINUS:
    case GEN_BINARY_DIVIDE:
	Index = binaryOp( node );
	break;

    case GEN_UNARY_MINUS:
	temp = getExprInReg(gen_UNARY_MINUS_get_rvalue(node));
	temp_type = fst_my_GetFieldByIndex(ft_SymTable, temp, SYMTAB_TYPE);
	zero = getConstantInRegFromString("0", TYPE_INTEGER, temp_type);
	Index = TempReg(zero, temp, iSUB, temp_type);
	generate(0, subs_by_type[index_by_type(temp_type)], zero,
			temp, Index, "Unary minus");
	break;

    case GEN_BINARY_LT:
        Index = relOp( node, LT );
	break;
    case GEN_BINARY_LE:
        Index = relOp( node, LE );
	break;
    case GEN_BINARY_EQ:
        Index = relOp( node, EQ );
	break;
    case GEN_BINARY_NE:
        Index = relOp( node, NE );
	break;
    case GEN_BINARY_GE:
        Index = relOp( node, GE );
	break;
    case GEN_BINARY_GT:
        Index = relOp( node, GT );
	break;

    case GEN_BINARY_AND:
    case GEN_BINARY_OR:
    case GEN_BINARY_EQV:
    case GEN_BINARY_NEQV:
	Index = booleanOp( node );
	break;

    case GEN_INVOCATION:
        Index = HandleIntrinsic(node);
	if (Index == SYM_INVALID_INDEX)
	  Index = HandleInvocation(node);
	break;

    case GEN_UNARY_NOT:
	Index = HandleUnaryNot(node);
	break;

    case GEN_BINARY_EXPONENT:
	Index = HandleExponent(node);
	break;

    case GEN_LABEL_REF:
    default:
	UNIMPL( node_type );
	Index = 0;
	break;
  }
  type = gen_get_converted_type(node);
  if (fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE) != type)
     Index = getConversion(Index, type);  

  return Index;
} /* getExprInReg */




/* converts value in register to specified type */
int getConversion( int reg, int type )
  // int reg, type;
{
  int Index, reg_type, op;

  if (aiDebug > 1)
     (void) fprintf(stdout, "getConversion( r%d, %d <%s>)\n", 
	     reg, type, TypeName(type));

  /* retrieve the type of the variable to be converted */
  reg_type = fst_my_GetFieldByIndex(ft_SymTable, reg, SYMTAB_TYPE);

  /* return if no conversion is required */
  if (reg_type == type)
    return reg;

  /* generate a register to hold the converted value */
  (void) sprintf(name_buffer, "C%d(%d)", reg, type);
  Index = SymInsertSymbol(name_buffer, type, OC_IS_DATA, 0,
			  SC_NO_MEMORY, NO_ALIAS);

  /* generate the appropriate conversion */
  switch (reg_type)
  {
  	case TYPE_INTEGER:
  	  switch (type)
  	  {
  	  	case TYPE_REAL:
  	  		op = i2f;	break;
  	  	case TYPE_DOUBLE_PRECISION:
  	  		op = i2d;	break;
  	  	case TYPE_COMPLEX:
  	  		op = i2c;	break;
  	  	case TYPE_DOUBLE_COMPLEX:
  	  		op = i2q;	break;
  	  	default:
  	  		op = ERR;	break;
  	  }
  	  break;
  	case TYPE_REAL:
  	  switch (type)
  	  {
  	  	case TYPE_INTEGER:
  	  		op = f2i;	break;
  	  	case TYPE_DOUBLE_PRECISION:
  	  		op = f2d;	break;
  	  	case TYPE_COMPLEX:
  	  		op = f2c;	break;
  	  	case TYPE_DOUBLE_COMPLEX:
  	  		op = f2q;	break;
  	  	default:
  	  		op = ERR;	break;
  	  }
  	  break;
  	case TYPE_DOUBLE_PRECISION:
  	  switch (type)
  	  {
  	  	case TYPE_INTEGER:
  	  		op = d2i;	break;
  	  	case TYPE_REAL:
  	  		op = d2f;	break;
  	  	case TYPE_COMPLEX:
  	  		op = d2c;	break;
  	  	case TYPE_DOUBLE_COMPLEX:
  	  		op = d2q;	break;
  	  	default:
  	  		op = ERR;	break;
  	  }
  	  break;
  	case TYPE_COMPLEX:
  	  switch (type)
  	  {
  	  	case TYPE_INTEGER:
  	  		op = c2i;	break;
  	  	case TYPE_REAL:
  	  		op = c2f;	break;
  	  	case TYPE_DOUBLE_PRECISION:
  	  		op = c2d;	break;
  	  	case TYPE_DOUBLE_COMPLEX:
  	  		op = c2q;	break;
  	  	default:
  	  		op = ERR;	break;
  	  }
  	  break;
  	case TYPE_DOUBLE_COMPLEX:
  	  switch (type)
  	  {
  	  	case TYPE_INTEGER:
  	  		op = q2i;	break;
  	  	case TYPE_REAL:
  	  		op = q2f;	break;
  	  	case TYPE_DOUBLE_PRECISION:
  	  		op = q2d;	break;
  	  	case TYPE_COMPLEX:
  	  		op = q2c;	break;
  	  	default:
  	  		op = ERR;	break;
  	  }
  	  break;
  }
  
  if (aiDebug > 1)
    (void) fprintf(stdout,
		"\tgetConversion( r%d, %d) => %d <%s> using opcode %s(%d).\n", 
	        reg, type, Index,
		TypeName(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE)),
		iloc_mnemonic(op), op);

  if (op == ERR)
  {
	(void) sprintf(error_buffer,
		"Invalid type conversion (%s(%d) => %s(%d)) for r%d ignored.\n",
		TypeName(reg_type), reg_type, TypeName(type), type, reg);
	ERROR("getConversion", error_buffer, WARNING);
	return reg;
  }

  /* generate the conversion instruction */
  generate(0, op, reg, Index, 0, NOCOMMENT);
	
  return Index;
} /* getConversion */




/* generate iloc to get an expression in a register */
/* if it is an identifier, substring, or subscript  */
int getIdInRegister(AST_INDEX node)
  // AST_INDEX node;
{
  int Index, Index_type, AReg, DReg, node_type, Offset = 0;
  char *comment;

  if (aiDebug > 1)
      (void) fprintf(stdout, "getIdInRegister( %d ).\n", node);
  
  node_type = gen_get_node_type(node);
  switch(node_type)
  {
    case GEN_IDENTIFIER:
	Index = getIndex(node);
	Index_type = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);
	
	/* if its in a register, leave Index alone and return */
	if (!(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_STORAGE_CLASS)
		& SC_NO_MEMORY))
	{
	  AReg = getAddressInRegister( Index );	/* guarded above */
	  DReg = StrTempReg("!", AReg, Index_type);
   	  generate_load(DReg, AReg, Index_type, Index,"&unknown");
	  generate_move(Index, DReg, Index_type);
	}
	break;

    case GEN_SUBSCRIPT:
	Index = getIndex(gen_SUBSCRIPT_get_name(node));
	Index_type = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);
	comment = GenDepComment(node);
	if (aiOptimizeAddressCode)
	  if (DepInfoPtr(node)->AddressLeader != AST_NIL)
	    {

	      // if this is the first reference in the loop body of on 
	      // Address Equivalence Set, then generate the address arithmetic
	      // for the Address Leader (the base address).
	      
	      if (DepInfoPtr(node)->FirstInLoop == node)
		{
		  AReg  = getSubscriptLValue(DepInfoPtr(node)->AddressLeader);
		  DReg  = StrTempReg("!", AReg, Index_type);
		  ASTRegMap->MapAddEntry(DepInfoPtr(node)->AddressLeader,AReg);
		}
	      else
		AReg = ASTRegMap->MapToValue(DepInfoPtr(node)->AddressLeader);
	      
	      if ((Offset = DepInfoPtr(node)->Offset*GetDataSize(TYPE_INTEGER))
		  != 0)
		{
		  // create code to for address arithmetic that can be 
		  // peepholed to use register + offset addressing mode
		  
		  int OffsetReg = getConstantInRegFromInt(Offset);
		  int op = ArithOp(GEN_BINARY_PLUS,TYPE_INTEGER);
		  int TempIndex = TempReg(AReg, OffsetReg, op, TYPE_INTEGER);
		  generate(0, op, AReg, OffsetReg, TempIndex, NOCOMMENT);
		  AReg = TempIndex;
		  DReg  = StrTempReg("!", AReg, Index_type);
		}
	      else
		DReg  = StrTempReg("!", AReg, Index_type);
	    }
	  else
	    {
	      AReg  = getSubscriptLValue(node);
	      DReg  = StrTempReg("!", AReg, Index_type);
	    }
	else
	  {
	    AReg  = getSubscriptLValue(node);
	    DReg  = StrTempReg("!", AReg, Index_type);
	  }
        if (aiSpecialCache && DepInfoPtr(node)->UsePrefetchingLoad)
	  generate_pfload(DReg,AReg,DepInfoPtr(node)->PrefetchDistance,
			  DepInfoPtr(node)->PrefetchOffsetAST,
			  Index_type,Index,comment);
	else
	  generate_load( DReg, AReg, Index_type, Index, comment);
	free(comment);
	Index = DReg;
	break;

    case GEN_SUBSTRING:
	AReg  = getSubstringAddress(node);
	if (gen_get_node_type(gen_SUBSTRING_get_substring_name(node))
	    == GEN_SUBSCRIPT) 
	  Index = getIndex(gen_SUBSCRIPT_get_name(gen_SUBSTRING_get_substring_name(node)));
	else
	  Index = getIndex(gen_SUBSTRING_get_substring_name(node));
	DReg  = StrTempReg("!", AReg, fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE));
	generate_load( DReg, AReg, fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE),Index,"&unknown");
	Index = DReg;
	break;

    default:
        (void) sprintf(error_buffer,"GetIdInRegister( %d ): improper node type.\n",
		node);
	ERROR("GetIdInRegister", error_buffer, FATAL);
	break;
  }
  return Index;
} /* getIdInRegister */




/* generate iloc to put a constant in a register */
int getConstantRegister( AST_INDEX node )
  // AST_INDEX node;
{
  register int Index;
  char  complex_buffer[128];
  char 		*name;


  if (gen_get_node_type(node) != GEN_COMPLEX_CONSTANT)
  {
    name = ConstantName(gen_get_text(node), gen_get_real_type(node),
		name_buffer);    
    Index = fst_my_QueryIndex(ft_SymTable, name);
    if (Index == SYM_INVALID_INDEX)
    {
      Index = SymInsertSymbol(name, gen_get_real_type(node), OC_IS_DATA, 0, SC_CONSTANT, 
			      NO_ALIAS);
      //(void) sprintf(error_buffer, "'%s' not entered in Symbol Table", name);
      //ERROR("getConstantRegister", error_buffer, FATAL);
    }
  }
  else
  {
    (void) NameFromConstantNode( node, complex_buffer );
    name = ConstantName(complex_buffer, gen_get_real_type(node), name_buffer);
    Index = fst_my_QueryIndex(ft_SymTable, name);
    if (Index == SYM_INVALID_INDEX)
    {
      Index = SymInsertSymbol(name, gen_get_real_type(node), OC_IS_DATA, 0, SC_CONSTANT, 
			      NO_ALIAS);
      //(void) sprintf(error_buffer, "'%s' not entered in Symbol Table", name);
      //ERROR("getConstantRegister", error_buffer, FATAL);
    }
  }

  if (aiDebug > 1)
     (void) fprintf(stdout, "getConstantRegister( node: %d, r%d '%s' ).\n", 
	     node, Index,
	     (char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME));

  Index = getConstantInRegFromIndex(Index, gen_get_converted_type(node));

  return Index;  
} /* getConstantRegister */




/* generate iloc to put the integer value "val" in a register */
int getConstantInRegFromInt(int val)
  // int val;
{
  char buffer[20];
  (void) sprintf(buffer,"%d",val);
  return getConstantInRegFromString(buffer, TYPE_INTEGER, TYPE_INTEGER);
} /* getConstantInRegFromInt */




/* generate iloc to convert the value indicated by "name" of type */
/* "type" to type "converted_type" and put result in a register   */
int getConstantInRegFromString( char *name, int type, int converted_type )
  // char *name;
  // int  type, converted_type;
{
  int 	i;
  char	*p;

  p = ConstantName(name, type, name_buffer);
  i = fst_my_QueryIndex(ft_SymTable, p);
  if (i == SYM_INVALID_INDEX)
  {
    i = SymInsertSymbol(p, type, OC_IS_DATA, 0, SC_CONSTANT, 
			NO_ALIAS);
  }
  return getConstantInRegFromIndex(i, converted_type);
} /* getConstantInRegFromString */




/* generate iloc to convert the value associated with symbol table */
/* index "index" to "converted_type" and put result in a register  */
int getConstantInRegFromIndex( int Index, int converted_type )
  // int Index, converted_type;
{
  register int source_type, address;
  int	value, AReg, BReg;
  int	contents;

  if (aiDebug > 0)
     (void) fprintf(stdout, "GetConstantInRegFromIndex( %d ).\n", Index);

  source_type = fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_TYPE);

  if (source_type == TYPE_INTEGER)
  {
    (void) get_num((char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME),
		&value); /* stops at '[' */

    if (converted_type != TYPE_INTEGER)
	Index = TempReg(value, 0, iLDI, converted_type);

    switch(converted_type)
      {
	case TYPE_LOGICAL:
	case TYPE_INTEGER:
	  generate(0, iLDI, value, Index, GEN_NUMBER, NOCOMMENT);
	  break;
	case TYPE_REAL:
	  generate(0, fLDI, value, Index, GEN_NUMBER, NOCOMMENT);
	  break;
	case TYPE_DOUBLE_PRECISION:
	  generate(0, dLDI, value, Index, GEN_NUMBER, NOCOMMENT);
	  break;
	case TYPE_COMPLEX:
	  generate(0, cLDI, value, Index, GEN_NUMBER, NOCOMMENT);
	  break;
	case TYPE_DOUBLE_COMPLEX:
	  generate(0, qLDI, value, Index, GEN_NUMBER, NOCOMMENT);
	  break;
	default:
	  (void) sprintf(name_buffer, "Invalid converted type %s(%d)\n",
		TypeName(converted_type), converted_type);
	  ERROR("getConstantInRegFromIndex", name_buffer, WARNING);
	  break;
      }
  }
  else if (source_type == TYPE_LOGICAL)
  {
    if (strncmp((char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME),
		 ".true.", 6) == 0)
       generate(0, iLDI, 1, Index, GEN_NUMBER, ".true.");
    else
       generate(0, iLDI, 0, Index, GEN_NUMBER, ".false.");

    if (source_type != converted_type)
      Index = getConversion(Index, converted_type);
  }
  else /* loading a non-INTEGER constant is complicated */
  {
    /* get the address into a register */
    AReg = getConstantInRegFromInt(fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_offset));
    BReg    = aiStaticLabel();
    address = TempReg(AReg, BReg, iADD, TYPE_INTEGER);

    generate(0, iADD, AReg, BReg, address, "Add in static base");

    /*  changed to get fmin to work  */
    contents   = StrTempReg("!", address, source_type);

    (void) sprintf(error_buffer, "Loading %s constant '%s'", decodeType(Index),
		(char *) fst_my_GetFieldByIndex(ft_SymTable, Index, SYMTAB_NAME));
	generate(0, NOP, 0, 0, 0, error_buffer);
	
    generate_load (contents, address, source_type, Index,"&unknown");
    generate_move (Index, contents, source_type);

    if (source_type != converted_type)
      Index = getConversion(Index, converted_type);
  }

  return Index;
} /* getConstantInRegFromIndex */




/* generate iloc for a binary operation */
int binaryOp( AST_INDEX node )
  // AST_INDEX node;
{
  int 		Index, node_type;
  int 		lhs_index, lhs_type;
  int 		rhs_index, rhs_type;
  AST_INDEX	lhs;
  AST_INDEX	rhs;
  int		result_type;
  int		op;

  if (aiDebug > 1)
      (void) fprintf(stdout, "binaryOp( %d ).\n", node);
  

  node_type = gen_get_node_type(node);

  lhs = gen_BINARY_PLUS_get_rvalue1(node);
  rhs = gen_BINARY_PLUS_get_rvalue2(node);

  lhs_index = getExprInReg( lhs );
  rhs_index = getExprInReg( rhs );

  lhs_type = gen_get_converted_type(lhs);
  rhs_type = gen_get_converted_type(rhs);

  if (lhs_type != rhs_type)
  {
    /* need to handle the conversion */
    result_type = Table2(lhs_type, rhs_type);

    if (lhs_type != result_type)
       lhs_index = getConversion(lhs_index, result_type);
    if (rhs_type != result_type)
       rhs_index = getConversion(rhs_index, result_type);
  }
  else
     result_type = lhs_type;

  op = ArithOp(node_type, result_type);
  Index = TempReg(lhs_index, rhs_index, op, result_type);

  switch(node_type)
  {
    case GEN_BINARY_TIMES:
    case GEN_BINARY_PLUS:
    case GEN_BINARY_MINUS:
    case GEN_BINARY_DIVIDE:
	generate(0, op, lhs_index, rhs_index, Index, NOCOMMENT);
	break;

    default:
	UNIMPL( node_type );
	Index = 0;
	break;
  }
  return Index;
} /* binaryOP */




static int
  Ops[] = {	ERR, 
		iADD, fADD, cADD, dADD, qADD,
		iSUB, fSUB, cSUB, dSUB, qSUB,
		iMUL, fMUL, cMUL, dMUL, qMUL,
		iDIV, fDIV, cDIV, dDIV, qDIV  };




/* return the appropriate arithmetic opcode "op" of type "type" */ 
int ArithOp( int op, int type )
  // int op, type;
{
  int	c1, c2, result;

  switch(op)
  {
    case GEN_BINARY_PLUS:	c1 = 0;		break;
    case GEN_BINARY_MINUS:	c1 = 1;		break;
    case GEN_BINARY_TIMES:	c1 = 2;		break;
    case GEN_BINARY_DIVIDE:	c1 = 3;		break;
    default:			c1 = -1;	break;
  }

  switch(type)
  {
    case TYPE_CHARACTER:	c2 = -1;	break;
    case TYPE_LOGICAL:		c2 = -1;	break;
    case TYPE_INTEGER:		c2 = 1;		break;
    case TYPE_LABEL:		c2 = -1;	break;
    case TYPE_REAL:		c2 = 2;		break;
    case TYPE_COMPLEX:		c2 = 3;		break;
    case TYPE_DOUBLE_PRECISION:	c2 = 4;		break;
    case TYPE_DOUBLE_COMPLEX:	c2 = 5;		break;
    default:			c2 = -1;	break;
  }
  if (c1 == -1 || c2 == -1)
     result = 0;
  else 
     result = c1*5+c2;

  return Ops[result];
} /* ArithOp */




/* given the types associated with the operands, return the type   */
/* type of the result.  table 2 from X3J3/90 (77-06-20), page 6-5, */
/* modified to include double precision complex numbers, specifies */
/* conversions for arithmetic operations +,-,*,/		   */
int Table2( int type1, int type2 ) 
  // int type1, type2;
{
   register int value;
   switch (type1)
   {
     case TYPE_INTEGER:
	switch (type2)
 	{
	  case TYPE_INTEGER:		value = TYPE_INTEGER;		break;
	  case TYPE_REAL:		value = TYPE_REAL;		break;
	  case TYPE_DOUBLE_PRECISION:	value = TYPE_DOUBLE_PRECISION;	break;
	  case TYPE_COMPLEX:		value = TYPE_COMPLEX;		break;
	  case TYPE_DOUBLE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;	break;
	  default:
		ERROR("Table2", "invalid type conversion", FATAL);
		break;
	}
	break;
     case TYPE_REAL:
	switch (type2)
 	{
	  case TYPE_INTEGER:		value = TYPE_REAL;		break;
	  case TYPE_REAL:		value = TYPE_REAL;		break;
	  case TYPE_DOUBLE_PRECISION:	value = TYPE_DOUBLE_PRECISION;	break;
	  case TYPE_COMPLEX:		value = TYPE_COMPLEX;		break;
	  case TYPE_DOUBLE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;	break;
	  default:
		ERROR("Table2", "invalid type conversion", FATAL);
		break;
	}
	break;
     case TYPE_DOUBLE_PRECISION:
	switch (type2)
 	{
	  case TYPE_INTEGER:		value = TYPE_DOUBLE_PRECISION;	break;
	  case TYPE_REAL:		value = TYPE_DOUBLE_PRECISION;	break;
	  case TYPE_DOUBLE_PRECISION:	value = TYPE_DOUBLE_PRECISION;	break;
	  case TYPE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;		break;
	  case TYPE_DOUBLE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;	break;
	  default:
		ERROR("Table2", "invalid type conversion", FATAL);
		break;
	}
	break;
     case TYPE_COMPLEX:
	switch (type2)
 	{
	  case TYPE_INTEGER:		value = TYPE_COMPLEX;		break;
	  case TYPE_REAL:		value = TYPE_COMPLEX;		break;
	  case TYPE_DOUBLE_PRECISION:	value = TYPE_DOUBLE_COMPLEX;	break;
	  case TYPE_COMPLEX:		value = TYPE_COMPLEX;		break;
	  case TYPE_DOUBLE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;	break;
	  default:
		ERROR("Table2", "invalid type conversion", FATAL);
		break;
	}
	break;
     case TYPE_DOUBLE_COMPLEX:
	switch (type2)
 	{
	  case TYPE_INTEGER:		value = TYPE_DOUBLE_COMPLEX;	break;
	  case TYPE_REAL:		value = TYPE_DOUBLE_COMPLEX;	break;
	  case TYPE_DOUBLE_PRECISION:	value = TYPE_DOUBLE_COMPLEX;	break;
	  case TYPE_COMPLEX:		value = TYPE_DOUBLE_COMPLEX;	break;
	  case TYPE_DOUBLE_COMPLEX:	value = TYPE_DOUBLE_COMPLEX;	break;
	  default:
		ERROR("Table2", "invalid type conversion", FATAL);
		break;
	}
	break;

     default:
	ERROR("Table2", "invalid type conversion", FATAL);
	break;

   }
   return value;
} /* Table2 */
