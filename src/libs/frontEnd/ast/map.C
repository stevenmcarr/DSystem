/* $Id: map.C,v 1.5 1997/03/11 14:29:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <include/rn_varargs.h>

#include <libs/support/misc/general.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/treeutil.h>
#include <libs/support/msgHandlers/log_msg.h>

#include <libs/frontEnd/ast/map.h>

static void 
mapExpr(AST_INDEX id_node,  ReferenceAccessType, 
	va_list arg_list)
{
  FortTextTree        ftt = va_arg(arg_list, FortTextTree);
  FortTreeSideArray    md = va_arg(arg_list, FortTreeSideArray);
  char              *text = va_arg(arg_list, char *);
  int l1, l2, c1, c2;

  if ( id_node == AST_NIL ) return;

  id_node = ast_out_if_subscript_id(id_node);

  ftt_NodeToText(ftt, id_node, &l1, &c1, &l2, &c2);

  ft_PutToSideArray(md, id_node, 0, l1);
  ft_PutToSideArray(md, id_node, 1, c1);
  ft_PutToSideArray(md, id_node, 2, l2);
  ft_PutToSideArray(md, id_node, 3, c2);

  ft_PutToSideArray(md, id_node, 4, (Generic) text);
}

int mapStmt ( FortTreeNode stmt, int /* nesting_level */ , va_list arg_list )
{
  
  Boolean	 fullmap = va_arg(arg_list, Boolean);
  FortTextTree	     ftt = va_arg(arg_list, FortTextTree);
  FortTreeSideArray   md = va_arg(arg_list, FortTreeSideArray);
  int l1, l2, c1, c2;

  ftt_NodeToText(ftt, stmt, &l1, &c1, &l2, &c2);
  
  ft_PutToSideArray(md, stmt, 0, l1);
  ft_PutToSideArray(md, stmt, 2, l2);
  
  if (fullmap) {
    char *stmt_text = ftt_GetTextLine(ftt, l1);
    
    switch (gen_get_node_type(stmt)) {
      case GEN_ASSIGNMENT:
      case GEN_LOGICAL_IF:
      case GEN_ARITHMETIC_IF:
      case GEN_DO:
      case GEN_PARALLELLOOP:
      case GEN_CALL:
      case GEN_GUARD:
      case GEN_PRINT:
      case GEN_WRITE:
      case GEN_READ_SHORT:
      case GEN_READ_LONG:
      case GEN_COMPUTED_GOTO:
      case GEN_RETURN:
      ft_PutToSideArray(md, stmt, 4, (Generic) stmt_text);
      break;
      
      default:
      log_msg((Boolean) 000, "mapStmt(): unknown stmt type %d\n",
	      gen_get_node_type(stmt));
      break;
    }
    
    walkIDsInStmt(stmt, mapExpr, ftt, md, stmt_text);
  }
  
  return WALK_CONTINUE;
}
