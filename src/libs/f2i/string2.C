/* $Id: string2.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/f2i/mnemonics.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <include/frontEnd/astnode.h>
#include <libs/frontEnd/ast/gen.h>
#include <include/frontEnd/astsel.h>
#include <include/frontEnd/astrec.h>

#include <libs/f2i/f2i_label.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/ai.h>
#include <libs/f2i/char.h>
#include <libs/f2i/classes.h>

/* forward declarations */




/* handles the complexity of moving a character string */
void generate_move_string(struct CharDesc *Target,struct CharDesc *Source)
  //struct CharDesc *Target,*Source;
{
  int loop_index = 0;
  int next_stmt;
  int TLenReg, TAddrReg, BlankReg;
  int index, FunctionIndex;
  int MinLenReg;
  int NewTAddrReg,NewTLenReg;
  int SLenReg;
  int have_byte;
  AST_INDEX funcName;

  if (aiGenerate != 0)
  {
    return;
  }

  if (aiAnnotate)
  {
    generate(0, NOP, 0, 0, 0, "Start string move");
  }

  /* if two+ expressions in RHS, we need a label for end of moving */
  if (Source->addr != END_OF_CHAR_LIST && Source[1].addr != END_OF_CHAR_LIST)
    loop_index = LABEL_NEW;

  if (aiAnnotate)
    generate(0, NOP, 0, 0, 0, "Load target address & length");

  /* compute initial target length and address */
  switch (Target->description)
  {
  case CHAR_NORMAL:
    TLenReg = getConstantInRegFromInt(Target->misc);
    break;
  case CHAR_FUNCTION:
    ERROR("generate_string_move","Target of move is a function",FATAL);
    break;
  case CHAR_UNKNOWN_LEN:
    TLenReg = Target->misc;
    break;
  default:
    ERROR("generate_move_string","Target character description type unknown",FATAL);
    break;
  }

  /* TAddrReg should be a temporary */
  TAddrReg = StrTempReg("StrMov",Target->addr,TYPE_CHARACTER);
  generate_move(TAddrReg,Target->addr,TYPE_INTEGER);


  for (index = 0;Source[index].addr != END_OF_CHAR_LIST;index++)
  {

    if (aiAnnotate)
      generate(0, NOP, 0, 0, 0, "Move one expression");

    /* Choose source length and copy */
    have_byte = -1;
    switch (Source[index].description)
    {
    case CHAR_NORMAL:
      SLenReg = getConstantInRegFromInt(Source[index].misc);
      MinLenReg = TempReg(TLenReg,SLenReg,iMIN,TYPE_INTEGER);
      generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
      generate_mvcl(Source[index].addr, TAddrReg, MinLenReg);
      break;
    case CHAR_FUNCTION:
      /* the way functions work:                          */
      /* we should know the length of the function at     */
      /* compile time and have already allocated stack    */
      /* space for the function.  We pass (1) the address */
      /* of this space and (2) the length of this space.  */
      /* Then we load the source length                   */

      MinLenReg = TempReg(TLenReg,SLenReg,iMIN,TYPE_INTEGER);
      generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
      funcName = gen_INVOCATION_get_name((AST_INDEX) Source[index].misc);
	
      if (iloc_intrinsic(gen_get_text(funcName)) != A2I_INVALID_OPCODE)
      {
	have_byte = HandleIntrinsic((AST_INDEX) Source[index].misc);
	SLenReg = getIntConstantInRegister("1");
	MinLenReg = TempReg(TLenReg, SLenReg, iMIN,TYPE_INTEGER);
	generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
	generate_store(TAddrReg, have_byte, TYPE_CHARACTER,-1,"&unknown");
      }
      else if (fst_my_GetFieldByIndex(ft_SymTable, getIndex(funcName),
		SYMTAB_STORAGE_CLASS) & SC_STMT_FUNC)
      {
	int SAddrReg;
	
	SAddrReg = InlineStmtFunc((AST_INDEX) Source[index].misc);
	SLenReg = getConstantInRegFromInt
		(fst_my_GetFieldByIndex(ft_SymTable, getIndex(funcName), SYMTAB_CHAR_LENGTH));
	MinLenReg = TempReg(TLenReg, SLenReg, iMIN, TYPE_INTEGER);
	generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
	generate_mvcl(SAddrReg, TAddrReg, MinLenReg);
      }
      else
      {
	(void) HandleInvocation((AST_INDEX) Source[index].misc);
	SLenReg = getStringLengthIntoReg((AST_INDEX) Source[index].misc);
	FunctionIndex = fst_my_QueryIndex(ft_SymTable,
		gen_get_text(gen_INVOCATION_get_name((AST_INDEX) Source[index].misc)));
	Source[index].addr = getFunctionAddressInReg(FunctionIndex);
	MinLenReg = TempReg(TLenReg, SLenReg, iMIN, TYPE_INTEGER);
	generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
	generate_mvcl(Source[index].addr, TAddrReg, MinLenReg);
      }
      break;
    case CHAR_UNKNOWN_LEN:
      SLenReg = Source[index].misc;
      MinLenReg = TempReg(TLenReg, SLenReg, iMIN, TYPE_INTEGER);
      generate(0, iMIN, TLenReg, SLenReg, MinLenReg, "Find min(TargetLen,SourceLen)");
      generate_mvcl(Source[index].addr, TAddrReg, MinLenReg);
      break;
    default:
      ERROR("generate_string_move","Character description type unknown",FATAL);
      break;
    }
    
    /* recompute Target length */
    NewTLenReg = TempReg(TLenReg, MinLenReg, iSUB, TYPE_INTEGER);
    generate(0, iSUB, TLenReg, MinLenReg, NewTLenReg, "TargetLen -= min(TargetLen,SourceLen)");
    TLenReg = NewTLenReg;

    /* recompute Target Address */
    NewTAddrReg = TempReg(TAddrReg, MinLenReg, iADD, TYPE_INTEGER);
    generate(0, iADD, NewTAddrReg, TAddrReg, MinLenReg, "TargetAddr += min(TLen,SLen)");
    TAddrReg = NewTAddrReg;

    /* skip remaining copy instructions if done */
    if (Source[index+1].addr != END_OF_CHAR_LIST)
      generate_branch(0, LE, TLenReg, MinLenReg, TYPE_INTEGER,
	loop_index, NO_TARGET, NOCOMMENT);
    
    /* prepare for next execution of loop */
    TLenReg = NewTLenReg;
  }

  if (loop_index)
    generate(loop_index, NOP, 0, 0, 0, "Done moving expressions");
  
  /* fill in remaining blanks */
  loop_index = LABEL_NEW;
  next_stmt = LABEL_NEW;
  generate_branch(0, LE, TLenReg, (int) getIntConstantInRegister("0"), TYPE_INTEGER,
	loop_index, next_stmt, NOCOMMENT);
  generate(next_stmt, NOP, 0, 0, 0, "Target of false branch");
  BlankReg = getConstantInRegFromInt(BLANK_CHARACTER);
  generate_fill(TAddrReg, BlankReg, TLenReg);
  generate(loop_index, NOP, 0, 0, 0, "End of string move");


} /* generate_move_string */




/* compute the address of a substring */
int getSubstringAddress( AST_INDEX node )
//   AST_INDEX node;
{
  int VarAddr, OffsetAddr, TReg, AddrReg, OneReg;
  AST_INDEX temp;

  generate(0, NOP, 0, 0, 0, "Get Substring Address");
  
  /* first, get the address of the variable */
  if (gen_get_node_type(temp = gen_SUBSTRING_get_substring_name(node)) 
      == GEN_SUBSCRIPT)
  {
    VarAddr = getSubscriptLValue(temp);
  }
  else
  {
    VarAddr = getAddressInRegister(getIndex(temp));
  }

  /* now, get the offset -- defaults to 1 */
  if (gen_SUBSTRING_get_rvalue1(node) != ast_null_node)
    OffsetAddr = getExprInReg(gen_SUBSTRING_get_rvalue1(node));
  else
    OffsetAddr = getIntConstantInRegister("1");

  /* finally, add them up */
  TReg = TempReg(VarAddr, OffsetAddr, iADD, TYPE_INTEGER);
  generate(0, iADD, VarAddr, OffsetAddr, TReg, "Add substring offset");
  
  /* subtract one from result -- one indexed strings */
  OneReg = getIntConstantInRegister("1");
  AddrReg = TempReg(TReg, OneReg, iSUB, TYPE_INTEGER);
  generate(0, iSUB, TReg, OneReg, AddrReg, NOCOMMENT);
  
  return AddrReg;
} /* getSubstringAddress */




/* the length is always computed using iloc; if length */
/* is known at compile time, constant propagation will */
/* remove unneeded statements                          */
int getSubstringLength(AST_INDEX node)
  // AST_INDEX node;
{
  AST_INDEX son1,son2;
  int Reg1,Reg2;
  int TReg, OneReg, AddrReg, LastReg;
  AST_INDEX varName;
  int varIndex;
  
  /* son1 and son2 are actually son2 and son3 of */
  /* node.  However, they are the first and second */
  /* portions of the substring index */

  generate(0, NOP, 0, 0, 0, "get substring length");
  
  varName = gen_SUBSTRING_get_substring_name(node);
  if (gen_get_node_type(varName) == GEN_SUBSCRIPT)
    varName = gen_SUBSCRIPT_get_name(varName);
  varIndex = fst_my_QueryIndex(ft_SymTable, gen_get_text(varName));

  son1 = gen_SUBSTRING_get_rvalue1(node);
  if (son1 != ast_null_node)
    Reg1 = getConversion(getExprInReg(son1), TYPE_INTEGER);
  else
    /* default starting point in 1 */
    Reg1 = getIntConstantInRegister("1");

  son2 = gen_SUBSTRING_get_rvalue2(node);
  if (son2 != ast_null_node)
    Reg2 = getConversion(getExprInReg(son2), TYPE_INTEGER);
  else
  {
    /* lookup length in tables */
    Reg2 = fst_my_GetFieldByIndex(ft_SymTable, varIndex, SYMTAB_CHAR_LENGTH);
    if (Reg2 == STAR_LEN)
      Reg2 = StrTempReg("s-len", varIndex, TYPE_INTEGER);
    else
      Reg2 = getConstantInRegFromInt(Reg2);
  }
  TReg = TempReg(Reg1, Reg2, iSUB, TYPE_INTEGER);
  OneReg = getIntConstantInRegister("1");
  AddrReg = TempReg(TReg, OneReg, iADD, TYPE_INTEGER);
  
  /* for various reasons, we need the symbol table entry */
  /* to start with "s-len"				 */
  LastReg = StrTempReg("s-len", AddrReg, TYPE_INTEGER);

  if (aiGenerate == 0)
  {
    generate(0, iSUB, Reg2, Reg1, TReg, NOCOMMENT);
    generate(0, iADD, TReg, OneReg, AddrReg, NOCOMMENT);
    generate_move(LastReg, AddrReg, TYPE_INTEGER);
  }

  generate(0, NOP, 0, 0, 0, "Substring length loaded");

  return LastReg;
} /* getSubstringLength */




/* given a node of type character, load the length of that */
/* expression into a register and return that register     */
int getStringLengthIntoReg(AST_INDEX node)
  //AST_INDEX node;
{
  int Reg1, Reg2, ReturnReg;
  int index;
  char buffer[200];

  switch (gen_get_node_type(node))
  {
  case GEN_CONSTANT:
    index = getIndex(node);
    ReturnReg = getConstantInRegFromInt(StringLength(index));
    break;
  case GEN_IDENTIFIER:
    index = getIndex(node);
    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) != TYPE_CHARACTER)
    {
      (void) sprintf(buffer,
              "Attempt to take string length of '%s' which has type '%s'",
              (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		TypeName(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)));
      ERROR("getStringLengthIntoReg",buffer,FATAL);
    }
    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH) != STAR_LEN)
    {
      ReturnReg = getConstantInRegFromInt
	(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH));
    }
    else
    {
      ReturnReg = StrTempReg("s-len", index, TYPE_INTEGER);
    }
    break;
  case GEN_SUBSTRING:
    ReturnReg = getSubstringLength(node);
    break;
  case GEN_SUBSCRIPT:
    ReturnReg = getStringLengthIntoReg(gen_SUBSCRIPT_get_name(node));
    break;
  case GEN_INVOCATION:
    ReturnReg = getStringLengthIntoReg(gen_INVOCATION_get_name(node));
    break;
  case GEN_BINARY_CONCAT:
    Reg1 = getStringLengthIntoReg(gen_BINARY_CONCAT_get_rvalue1(node));
    Reg2 = getStringLengthIntoReg(gen_BINARY_CONCAT_get_rvalue2(node));
    ReturnReg = TempReg(Reg1, Reg2, iADD, TYPE_INTEGER);
    generate(0, iADD, Reg1, Reg2, ReturnReg, NOCOMMENT);
    break;
  default:
    (void) sprintf(buffer, "node %d of unknown type", node);
    ERROR("getStringLengthIntoReg", buffer, FATAL);
    break;
  }
  return(ReturnReg);
} /* getStringLengthIntoReg */




/* store string length information into the symbol table */
void AddStringLengthRegister(int index, int storageClass)
  //int index, storageClass;
{
  int reg;

  reg = StrTempReg("s-len", index, TYPE_INTEGER);
  fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_OBJECT_CLASS,  OC_IS_DATA);
  fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_STORAGE_CLASS,  storageClass);

  /* equality rules out ILOC_PARAMETER */
  if (storageClass == SC_STACK) {
    ERROR("AddStringLengthRegister", "I shouldn't be here", WARNING);
    fst_my_PutFieldByIndex(ft_SymTable, reg, SYMTAB_offset,  aiNextStack);
    aiNextStack += Align(VarSize(reg));
  }
} /* AddStringLengthRegister */




/* return the length of a string or STAR_LEN if it   */
/* or any part of it is STAR_LEN (with the exception */
/* of parameter statement constants                  */
int NewStringLength( AST_INDEX node )
  // AST_INDEX node;
{
  int sum = 0;
  int index;
  char buffer[200];

  switch (gen_get_node_type(node))
  {
  case GEN_CONSTANT:
    index = getIndex(node);
    sum = StringLength(index);
    break;
  case GEN_IDENTIFIER:
    index = getIndex(node);
    if (fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE) != TYPE_CHARACTER) {
      (void) sprintf(buffer,
              "Attempt to take string length of '%s' which has type '%s'",
              (char *) fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_NAME),
		TypeName(fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_TYPE)));
      ERROR("getStringLengthIntoReg", buffer, FATAL);
    }
    sum = fst_my_GetFieldByIndex(ft_SymTable, index, SYMTAB_CHAR_LENGTH);
    break;
  case GEN_SUBSTRING:
    sum = NewStringLength(gen_SUBSTRING_get_substring_name(node));
    break;
  case GEN_SUBSCRIPT:
    sum = NewStringLength(gen_SUBSCRIPT_get_name(node));
    break;
  case GEN_INVOCATION:
    sum = NewStringLength(gen_INVOCATION_get_name(node));
    break;
  case GEN_BINARY_CONCAT:
    sum = NewStringLength(gen_BINARY_CONCAT_get_rvalue1(node));
    if (sum != STAR_LEN)
      sum += NewStringLength(gen_BINARY_CONCAT_get_rvalue2(node));
    break;
  default:
    (void) sprintf(buffer, "node %d of unknown type", node);
    ERROR("getStringLengthIntoReg", buffer, FATAL);
    break;
  }
  return(sum);
} /* NewStringLength */




/* inline the code for a character statement function */
static int InlineCharStmtFunc(AST_INDEX node)

  // AST_INDEX	node;

{
  AST_INDEX	list;
  int		counter = 0;
  int		index;
  int		function;
  char 		name[32];
  int           dummy;
  struct CharDesc target[2],source[MAX_CONCATS];
  
  /*  print debugging error message  */
  if (aiDebug > 0)
    (void) fprintf(stdout, "InlineCharStmtFunc( %d ).\n", node);
  
  /*  get the function information  */
  function = getIndex(gen_INVOCATION_get_name(node));
  
  /*  evaluate parameters in the parameter list  */
  list = list_first(gen_INVOCATION_get_actual_arg_LIST(node));
  
  while (list != ast_null_node)
  {
    /* move character expression onto stack and store address in Reg */
    createStackTarget(target,
	fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_CHAR_LENGTH));
    dummy = 0;
    evalCharExpr(list, source, &dummy, MAX_CONCATS);
    generate_move_string(target,source);

    (void) sprintf(name, "$%s[%d]",
	(char *) fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_NAME),
	counter++);
    index = fst_my_QueryIndex(ft_SymTable, name);
    if (index != SYM_INVALID_INDEX)
    {
      generate_move(index, target[0].addr, TYPE_INTEGER);
    }
    else
    {
      (void) sprintf(error_buffer, "%s called with too many parameters", 
	(char *) fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_NAME));
      ERROR("InlineStmtFunc", error_buffer, FATAL);
    }
    list = list_next(list);
  }

  /*  verify that all of the parameters were set  */
  if (-fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_addressReg) > counter)
  {
    (void) sprintf(error_buffer, "%s called with too few parameters", 
	(char *) fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_NAME));
    ERROR("InlineStmtFunc", error_buffer, FATAL);
  }
  
  /* finally, evaluate the expression and store the result on the stack */
  createStackTarget(target,
	fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_CHAR_LENGTH));
  dummy = 0;
  evalCharExpr((AST_INDEX) fst_my_GetFieldByIndex(ft_SymTable, function, SYMTAB_offset),
	source, &dummy, MAX_CONCATS);
  generate_move_string(target, source);
  return(target[0].addr);
} /* InlineCharStmtFunc */




/* creates space on the stack to store the "target"       */
/* character string during character string manipulations */
void createStackTarget(struct CharDesc target[2],int length)
// struct CharDesc target[2];
// int length;
{
  int offset = getConstantInRegFromInt(aiNextStack);
  target[0].addr = TempReg(offset, 0, iADD, TYPE_INTEGER);
  generate(0, iADD, offset, 0, target[0].addr, NOCOMMENT);
  target[0].misc = length;
  target[0].description = CHAR_NORMAL;
  target[1].addr = END_OF_CHAR_LIST;
  aiNextStack += Align(length);
} /* createStackTarget */
