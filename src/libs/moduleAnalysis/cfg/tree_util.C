/* $Id: tree_util.C,v 1.1 1997/06/25 15:03:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
#include <sys/types.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/moduleAnalysis/cfg/tree_util.h>

/*
 *  return control node (inductive, repetitive, or conditional)
 *  for any loop statement (do, doall, parallelloop)
 */
AST_INDEX gen_get_control(AST_INDEX stmt)
{
    switch (gen_get_node_type(stmt))
    {
      case GEN_DO:
        return (gen_DO_get_control(stmt));

      case GEN_DO_ALL:
        return (gen_DO_ALL_get_control(stmt));

      case GEN_PARALLELLOOP:
        return (gen_PARALLELLOOP_get_control(stmt));

      default:
	return AST_NIL;
    }
}




/*
 *  return label_ref node for any loop statement
 */
AST_INDEX gen_get_lbl_ref(AST_INDEX node)
{
    switch (gen_get_node_type(node))
    {
      case GEN_ASSIGN:
        return (gen_ASSIGN_get_lbl_ref(node));

      case GEN_AT:
	return (gen_AT_get_lbl_ref(node));

      case GEN_ERR_SPECIFY:
        return (gen_ERR_SPECIFY_get_lbl_ref(node));

      case GEN_END_SPECIFY:
        return (gen_END_SPECIFY_get_lbl_ref(node));

      case GEN_DO:
        return (gen_DO_get_lbl_ref(node));

      case GEN_DO_ALL:
        return (gen_DO_ALL_get_lbl_ref(node));

      case GEN_GOTO:
        return (gen_GOTO_get_lbl_ref(node));

      case GEN_PARALLELLOOP:
        return (gen_PARALLELLOOP_get_lbl_ref(node));

      case GEN_RETURN_LABEL:
        return (gen_RETURN_LABEL_get_lbl_ref(node));

      case GEN_STOPLOOP:
        return (gen_STOPLOOP_get_lbl_ref(node));
	
      default:
	return AST_NIL;
    }
}
