/* $Id: directives.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/Memoria/annotate/DirectivesInclude.h>

void HandleDirective(AST_INDEX Stmt)

  // AST_INDEX Stmt;

  {
   int Index,Reg;

   Index = fst_QueryIndex(ft_SymTable,
	     gen_get_text(gen_SUBSCRIPT_get_name(GET_DIRECTIVE_INFO(Stmt)->Subscript)));
   Reg = getSubscriptLValue(GET_DIRECTIVE_INFO(Stmt)->Subscript);
   generate_cache_op(Reg,Index,GET_DIRECTIVE_INFO(Stmt));
  }
