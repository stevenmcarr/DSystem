/* $Id: directives.C,v 1.3 1999/04/22 14:30:37 carr Exp $ */
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
   int Index,Reg;

   switch(GET_DIRECTIVE_INFO(Stmt)->Instr)
     {
      case PrefetchInstruction:
      case FlushInstruction:
	Index = fst_QueryIndex(ft_SymTable,
		gen_get_text(gen_SUBSCRIPT_get_name(GET_DIRECTIVE_INFO(Stmt)
						    ->Subscript)));
	if (aiOptimizeAddressCode)
	  if (GET_DIRECTIVE_INFO(Stmt)->AddressLeader != AST_NIL)
	    if (GET_DIRECTIVE_INFO(Stmt)->AddressLeader == 
		GET_DIRECTIVE_INFO(Stmt)->Subscript)
	      {
		Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);
		ASTRegMap->MapAddEntry(GET_DIRECTIVE_INFO(Stmt)->
				       Subscript,Reg);
	      }
	    else
	      {
		Reg = ASTRegMap->MapToValue(GET_DIRECTIVE_INFO(Stmt)->
					    AddressLeader);
		int Offset = GET_DIRECTIVE_INFO(Stmt)->Offset*
		  GetDataSize(TYPE_INTEGER);
		int OffsetReg = getConstantInRegFromInt(Offset);
		int op = ArithOp(GEN_BINARY_PLUS,TYPE_INTEGER);
		int TempIndex = TempReg(Reg, OffsetReg, op, TYPE_INTEGER);
		generate(0, op, Reg, OffsetReg, TempIndex, NOCOMMENT);
		Reg = TempIndex;
	      }
	  else
	    Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);
	else
	  Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);

	generate_cache_op(Reg,Index,GET_DIRECTIVE_INFO(Stmt));
	break;

      case SetSLRInstruction:
	{
	  int TempIndex = TempReg(GET_DIRECTIVE_INFO(Stmt)->SpecialLoadStride,
				  0,SETSLR,TYPE_INTEGER);
	  generate(0, SETSLR, GET_DIRECTIVE_INFO(Stmt)->SpecialLoadStride,
		   Index,GEN_NUMBER,"Set Spatial Load Stride");
	  break;
	}
     }
  }












