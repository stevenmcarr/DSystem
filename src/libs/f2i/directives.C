/* $Id: directives.C,v 1.5 1999/07/22 18:06:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/mnemonics.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>

void HandleDirective(AST_INDEX Stmt)

  // AST_INDEX Stmt;

  {
   int Index,Reg,Offset;

   switch(GET_DIRECTIVE_INFO(Stmt)->Instr)
     {
      case PrefetchInstruction:
      case FlushInstruction:
	Index = fst_QueryIndex(ft_SymTable,
		gen_get_text(gen_SUBSCRIPT_get_name(GET_DIRECTIVE_INFO(Stmt)
						    ->Subscript)));
	if (aiOptimizeAddressCode)
	  if (GET_DIRECTIVE_INFO(Stmt)->AddressLeader != AST_NIL)
	    {

	   // if this is the first reference in the loop body of on 
	   // Address Equivalence Set, then generate the address arithmetic
	   // for the Address Leader (the base address).

	      if (GET_DIRECTIVE_INFO(Stmt)->FirstInLoop == 
		  GET_DIRECTIVE_INFO(Stmt)->Subscript)
		{
		  Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->
					   AddressLeader);
		  ASTRegMap->MapAddEntry(GET_DIRECTIVE_INFO(Stmt)->
					 AddressLeader,Reg);
		}
	      else
		Reg = ASTRegMap->MapToValue(GET_DIRECTIVE_INFO(Stmt)->
					    AddressLeader);

	      if ((Offset = GET_DIRECTIVE_INFO(Stmt)->Offset
		   * GetDataSize(TYPE_INTEGER)) != 0)
		{
		  int OffsetReg = getConstantInRegFromInt(Offset);
		  int op = ArithOp(GEN_BINARY_PLUS,TYPE_INTEGER);
		  int TempIndex = TempReg(Reg, OffsetReg, op, TYPE_INTEGER);
		  generate(0, op, Reg, OffsetReg, TempIndex, NOCOMMENT);
		  Reg = TempIndex;
		}
	      }
	  else
	    Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);
	else
	  Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);

	generate_cache_op(Reg,Index,GET_DIRECTIVE_INFO(Stmt));
	break;

     }
  }












